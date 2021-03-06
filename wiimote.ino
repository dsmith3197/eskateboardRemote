/*
  Wiimote Electric Longboard Script 
*/

/*************************** CONFIGURATION ****************************/

#define DEBUGMODE //Comment this out to turn off debug messages and use of serial

/****************************** RECEIVER ******************************/
// libraries for nrf24l01 receiver
#include <SPI.h>
#include "RF24.h"
#include "nRF24L01.h"


// initialize addresses for the two receivers
const uint64_t ADDRESSES[] = {0xE8E9F3F6A4LL, 0xE9C2F0F0E1LL};
// define remote to be the second address
const bool radioNumber = 1;

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// initialize the delay between messages
const int DELAY = 250;

/******************************** INPUTS ********************************/
// set button pin numbers:
const int SAFETYBUTTONPIN = 7;
const int BRAKEBUTTONPIN = 6;
// declare variables for button states:
int safetyState6 = 0;      
int brakeState7 = 0;

// joystick constants
const int NEUTRAL = 502;
const int DECEL = 1023;
const int ACCEL = 0;
// joystick setup 
const int JOYSTICKPIN = 7;
// initialize joystick value to resting state
int joystickState = NEUTRAL; 
int safetyState = LOW;

// initialize datastructure to transmit data
struct dataStruct{
  unsigned long _micros;
  int joystick;
  int safety;
  int brake;
}myData;

/************************* END CONFIGURATION **************************/


/******************************* SETUP ********************************/
void setup() {
  
  #ifdef DEBUGMODE
    Serial.begin(115200);
    Serial.println("Remote is starting up");
  #endif
  
  /******** RADIO INITIALIZATION *******/
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  /* Set the PA Level low to prevent power supply related issues since the
   * likelihood of close proximity of the devices. RF24_PA_MAX is default.
   * LIKELY TO CHANGE
   */
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(ADDRESSES[1]);
  // ensure radio is not listening and ready to send data
  radio.stopListening();
  
  /********* BUTTON INITIALIZATION ********/
  // initialize the button pins as inputs:
  pinMode(SAFETYBUTTONPIN, INPUT_PULLUP);
  pinMode(BRAKEBUTTONPIN, INPUT_PULLUP);
  
  // initialize values in myData
  myData.joystick = NEUTRAL;
  myData.safety = LOW;
  myData.brake = LOW;
}
/***************************** END SETUP ******************************/


/******************************* LOGIC ********************************/
void loop() {
  // update data with new values of buttons and joystick
  myData._micros = millis();
  // read in state of buttons
  myData.brake = digitalRead(BRAKEBUTTONPIN);
  myData.safety = digitalRead(SAFETYBUTTONPIN);
  myData.joystick = analogRead(JOYSTICKPIN);
  
  // send data
  if (radio.write( &myData, sizeof(myData) )){
    #ifdef DEBUGMODE
      Serial.println(F("sent"));
    #endif
    delay(1);
  }
  else{
    #ifdef DEBUGMODE
      Serial.println(F("failed"));
    #endif
    delay(1);
  }
  // delay
  delay(DELAY);
}
