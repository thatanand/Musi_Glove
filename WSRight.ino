/*
Welcome to MusiGlove!!!
 
 The musical device perfect for those who just want to have fun without having to have any music experience
 
 The controls are simple. Just use your thumb to trigger sound samples on each of the other 4 fingers.
 Use the potentiometer to control the volume of the sounds and the reset button if anything goes wrong.
 
 For use of this code, please download the following libraries : CapSense and WaveHC
 
 Happy music-making!
 
 Authors: Anand Subramaniam, John Perez Chicaiza and Lehang Tieu
 */


/* Here we add the external libraries for the WaveShield, including those required to utilise the SD card module as well as the CapacitiveSensor library*/
#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"
#include <CapacitiveSensor.h>

/* We create objects that hold information. These objects are part of built in classes in the WaveShield Library*/
SdReader card;
FatVolume vol;
FatReader root;
FatReader f; 
/*The waveshield can only play a file at time, therefore we create on one audio object from the built in WaveHC class*/
WaveHC wave;

/*We are using capactive sensors to trigger the sounds below. Therefore, we define them here*/
const int sendPin = A0;
const int sensor1 = A1;
const int sensor2 = A2;
const int sensor3 = A3;
const int sensor4 = A4;

/*Define LED pins for visual feedback: red for errors, green for none*/
const int redLED = 8;
const int greenLED = 7;

/*Using the capactiveSensor Library we create new instances for each sensor, but each having one common send pin*/
CapacitiveSensor   cs_0_1 = CapacitiveSensor(sendPin,sensor1);
CapacitiveSensor   cs_0_2 = CapacitiveSensor(sendPin,sensor2); 
CapacitiveSensor   cs_0_3 = CapacitiveSensor(sendPin,sensor3); 
CapacitiveSensor   cs_0_4 = CapacitiveSensor(sendPin,sensor4);

/*This function checks whether there is a problem with the SDcard. This uses built in functions in the library and returns
the error.*/
void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

void setup() {
/*Add serial port for feedback when the sketch loads up initially, indicating the user that sketch loading has begun*/
  Serial.begin(9600);
  putstring_nl("Welcome to MusiGlove, 4 sensors with unlimited creativity");

 
/* These pins are for the DAC control which are reserved and defined in the Waveshield library*/
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
 
/*Set on and off LEDs as outputs*/
  pinMode(greenLED,OUTPUT);
  pinMode(redLED, OUTPUT);

 /*Turn red led on to show that the program is not ready yet, combined with a message for additional user feedback*/ 
  digitalWrite(redLED, HIGH);
  putstring_nl("Please bare with me, whilst I ready myself for playing");
 
/*This checks if SD card is inserted, if not the message will print and RED Led will show for visual feedback.
If this tests true, the user must make sure SD card is inserted and restart the code using the Reset button on the WaveShield*/
  if (!card.init()) {
    putstring_nl("Card initialisation failed!");
    sdErrorCheck();
    while(1){
     digitalWrite(redLED, HIGH);
     putstring_nl("Uh oh, something has seem to have gone wrong... Perhaps something is wrong with your SD card?");
     putstring_nl("After attempting to fix the problem, please press the Reset button");
    };    // then 'halt' - do nothing!
}

/*This optimizes card reading. It is an internal function in the Waveshield library*/
  card.partialBlockRead(true);
 
/*This checks the FAT partition for SD card to see if it's the right one or not. Again, this uses code from the library*/
  uint8_t part;
  for (part = 0; part < 5; part++) {
    if (vol.init(card, part)) 
      break;
  }
  if (part == 5) {
     putstring_nl("Uh oh, seems like there aren't any valid FAT partitions on your SD card!");
    sdErrorCheck();
    playcomplete("ERRFX.WAV");
    while(1);
  }
  
/*This prints all the information found, which are useful for user feedback in case of errors.*/
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(),DEC);
  
   // Try to open the root directory, if not an error message will be displayed
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root directory!");
    while(1);
  }
  
/*Inform the user that everything is good to go alongside some instructions*/
  putstring_nl("");
  putstring_nl("************************************************");
  putstring_nl("|||||||||| LET'S PLAY MUSIGLOVE!! ||||||||||||");
  putstring_nl("************************************************");
  putstring_nl("1. Use your 4 fingers to tap your thumb and generate sounds.");
  putstring_nl("2. Try to synchronize your sounds with what the other player is doing to make music!");
  
/*Play start up sound effect*/
  playcomplete("FXC.WAV");

/*Set ready light on (green LED) and some serial output for visual feedback*/
digitalWrite(greenLED, HIGH);
putstring_nl("Alright, let's make some music!");

  
}

void loop() {
/*Store the values or readings of the sensor accordingly, threshold is used to determine the sensitivity of the sensor. If a sensor is touched
and it meets the threshold, a sound will execute, along with the reading value of the sensor, respectively*/     
    long sensor1 =  cs_0_1.capacitiveSensor(30);
    long sensor2 =  cs_0_2.capacitiveSensor(30);
    long sensor3 =  cs_0_3.capacitiveSensor(30);
    long sensor4 =  cs_0_4.capacitiveSensor(30);

    int threshold = 200;
 
      if(sensor1>threshold){
        playfile("Kick.WAV");
        Serial.println(sensor1);
      }
      
      if(sensor2>threshold){
        playfile("Hihat.WAV");
        Serial.println(sensor2);
      }
      
      if(sensor3>threshold){
        playfile("Snare.WAV");
        Serial.println(sensor3);
      }
      
      if(sensor4>threshold){
        playfile("Cymbal.WAV");
        Serial.println(sensor4);
      }
}

/*This function plays a full file from beginning to end with no pause (library built function), if the name of the wave file is FXB.wav, 
the red LED is turned on and then off to indicate an error*/
void playcomplete(char *name) {
  playfile(name);
  while (wave.isplaying) {
    if(name == "FXC.WAV"){
      for(int i=0; i<10; i++){
      digitalWrite(redLED, HIGH);
      delay(100);
      digitalWrite(redLED, LOW); 
      delay(100);
      }
    }
  }
}

/*This function plays a file but can be overwritten to play other sounds with the use of an interrupt. However if the sketch has trouble loading the file
or if the WAV file is not valid, and error sound is played along with text and LED feedback*/
void playfile(char *name) {
  if (wave.isplaying) {
    wave.stop();
  }
  if (!f.open(root, name)) {
    putstring("Couldn't open file ");
    Serial.print(name);
    digitalWrite(redLED, HIGH);
    playcomplete("ERRFX.WAV");
    
    return;
  }
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV");
    digitalWrite(redLED, HIGH);
    playcomplete("ERRFX.WAV");
    return;
  }
  wave.play();

/*Trigger the green LED only if the file playing does not contain ERRFX,
because this sound is used only for errors and therefore uses only the RED LED light.
This will avoid triggering both lights at the same time.*/
  if(name!="ERRFX.WAV"){
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  delay(125);
  digitalWrite(greenLED, LOW);
  }

}
