/********Scout v1 : Arduino Controlled Obstacle Avoider that can talk!***
*************Coded and tested successfully by Sidharth Makhija***********
**************https://droidhangar.wordpress.com/contact/*****************
********************Released AS-IS under MIT Licence***************************
*************************************************************************
***********************************************************************/
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include <WaveUtil.h>
#include <WaveHC.h>
#include <ServoTimer2.h>

#define servoPin  6
ServoTimer2 servo1;

int trig = 16;
int echo =  17;
unsigned long pulsetime = 0;

//Vaariables where distances measured are stored.
int distFront;
int distLeft;
int distRight;
//Minimum distance of 20cm from any obstacle.
int distLimit = 20;

int in1 = 9;
int in2 = 19;
int in3 = 5;
int in4 = 3;
int blueLed = 15;
int redLed = 14;

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play

WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

#define DEBOUNCE 100  // button debouncer

// this handy function will return the number of bytes currently free in RAM, great for debugging!   
int freeRam(void)
{
  extern int  __bss_end; 
  extern int  *__brkval; 
  int free_memory; 
  if((int)__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__bss_end); 
  }
  else {
    free_memory = ((int)&free_memory) - ((int)__brkval); 
  }
  return free_memory; 
} 

void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

void setup() 
{
  // set up serial port
  Serial.begin(9600);
  putstring("Free RAM: ");       // This can help with debugging, running out of RAM is bad
  Serial.println(freeRam());      // if this is under 150 bytes it may spell trouble!
  
  servo1.attach(servoPin);
  
  
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);
  pinMode(trig,OUTPUT);
   pinMode(echo,INPUT);
 
  
  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)  
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while(1);                            // then 'halt' - do nothing!
  }
  
  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);
 
// Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part)) 
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while(1);                            // then 'halt' - do nothing!
  }
  
  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);     // FAT16 or FAT32?
  
  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while(1);                             // then 'halt' - do nothing!
  }
  
  // Whew! We got past the tough parts.
  putstring_nl("Ready!");
}

void loop() 
{
       stopMotor();
       servo1.write(1650);
       blue();
       Serial.println("Setup Complete");
       delay(3000);
       playcomplete("R2greet.WAV");
        playcomplete("R2start.WAV");       
       distFront = readDistance();
       Serial.print("Dist in front is = ");
         Serial.println(distFront);
       if(distFront<distLimit)
       {
         stopMotor();
         angry();
         delay(500);
         playcomplete("R2Det.WAV");
        servo1.write(2300);
         delay(500);
         distLeft = readDistance();
         Serial.print("Left Dist is = ");
         Serial.println(distLeft);
         delay(10);
         servo1.write(550);
         delay(500);
         distRight = readDistance();
         Serial.print("Right Dist is = ");
         Serial.println(distRight);
         delay(10);
         servo1.write(1650);
         delay(10);
         if(distLeft>distRight)
         {
           angry();
           playcomplete("R2a.WAV");
           delay(10);
           left();
           delay(1500);
         }
         else
         {           
           playcomplete("R2b.WAV");
           delay(10);
           right();
           delay(1500);
         }
       }
      
      else 
      {
        forward();
        Serial.println("Forward");
      }
}


//plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
  // do nothing while its playing
  }
  // now its done playing
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file "); Serial.print(name); return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV"); return;
  }
  
  // ok time to play! start playback
  wave.play();
}

void blue()
{
   digitalWrite(redLed,LOW);
  digitalWrite(blueLed,HIGH);
  
  
}


void forward()
{
  digitalWrite(in1,HIGH);
  digitalWrite(in2,LOW);
  digitalWrite(in3,HIGH);
  digitalWrite(in4,LOW);
  
}

void backward()
{
  digitalWrite(in1,LOW);
  digitalWrite(in2,HIGH);
  digitalWrite(in3,LOW);
  digitalWrite(in4,HIGH);
  
}

void right()
{
  digitalWrite(in1,HIGH);
  digitalWrite(in2,LOW);
  digitalWrite(in3,LOW);
  digitalWrite(in4,HIGH);
  
}

void left()
{
  digitalWrite(in1,LOW);
  digitalWrite(in2,HIGH);
  digitalWrite(in3,HIGH);
  digitalWrite(in4,LOW);
  
}

void stopMotor()
{
  digitalWrite(in1,LOW);
  digitalWrite(in2,LOW);
  digitalWrite(in3,LOW);
  digitalWrite(in4,LOW);
  
}

void angry()
{
  digitalWrite(blueLed,LOW);
   delay(10);
   digitalWrite(redLed,HIGH);
   delay(1000);
}

int readDistance()
{
  
  digitalWrite(trig, LOW); 
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);       
  delayMicroseconds(10);               
  digitalWrite(trig,LOW);         
  pulsetime = pulseIn(echo, HIGH);  
  return pulsetime/58.2;//magic                
                                       
}
