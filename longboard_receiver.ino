/*
  Longboard Receiver Script 
*/

/*************************** CONFIGURATION ****************************/

#define DEBUGMODE //Comment this out to turn off debug messages and use of serial

/******************************** ESC *********************************/
#include <Servo.h>
Servo ESC;
const int ESCPIN = 6;


/****************************** RECEIVER ******************************/
// libraries for nrf24l01 receiver
#include <SPI.h>
#include "RF24.h"
#include "nRF24L01.h" 

// initialize addresses for the two receivers
const uint64_t ADDRESSES[] = {0xE8E9F3F6A4LL, 0xE9C2F0F0E1LL};
// define longboard to be the first address
const bool radioNumber = 0;

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

/******************************** INPUTS ********************************/
// throttle constants
const int NEUTRAL = 502;
const int DECEL = 1023;
const int ACCEL = 0;

// initialize datastructure to transmit data
struct dataStruct{
  unsigned long _millis;
  int joystick;
  int safety;
  int brake;
}myData;

unsigned long timeOfLastMessage = millis();
const long TIMEOUT = 1000; 
bool timeout = false;

const int BRAKEDELAY = 150;
const int MAXSPEED = 140;
const int COASTSPEED = 100;
const int MINSPEED = 40;
const int ACCELERATION = 3;
int throttle = COASTSPEED;
bool brakePressed = false;

/************************* END CONFIGURATION **************************/

/******************************* SETUP ********************************/
void setup() {

  #ifdef DEBUGMODE
    Serial.begin(115200);
    Serial.println("Longboard is starting up");
  #endif
  
  /********* ESC INITIALIZATION ********/
  ESC.attach(ESCPIN);
  delay(100);
  // LIKELY TO CHANGE: arm ESC 
  ESC.write(COASTSPEED);
  
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
  radio.openReadingPipe(1,ADDRESSES[1]);
  // ensure longboard is listening and ready to receive data
  radio.startListening();
  
  // initialize values in myData
  myData.joystick = NEUTRAL;
  myData.safety = HIGH;
  myData.brake = HIGH;
}
/***************************** END SETUP ******************************/


/******************************* LOGIC ********************************/
void loop() {
  // ensure radio is available
  if(radio.available()){
    timeout = false;
    while (radio.available()) {                          // While there is data ready
      radio.read( &myData, sizeof(myData) );             // Get the payload
    }
      
    // update the time of the last message
    timeOfLastMessage = millis();    
    
    // update ESC
    if (myData.brake == LOW) {
      // brake
      brakePressed = true;
      throttle = COASTSPEED - 20;
      ESC.write(throttle);
      delay(BRAKEDELAY);
      while (throttle >= MINSPEED) {
        throttle -= ACCELERATION;
        ESC.write(throttle);
        delay(BRAKEDELAY);
      }
      delay(2000);
      ESC.write(COASTSPEED);

    }
    else if (myData.safety == HIGH && myData.joystick == 1023) {
      ESC.write(COASTSPEED);
    }
    else if (myData.safety == LOW) {
      // calculate speed increase
      throttle += map(myData.joystick, 0, 1023, -ACCELERATION, ACCELERATION) * -1 - 1;
      if (throttle > MAXSPEED) {
        throttle = MAXSPEED;
      }
      else if (throttle < COASTSPEED) {
        throttle = COASTSPEED;
      }
      ESC.write(throttle);
    }
    
    #ifdef DEBUGMODE
      Serial.print(F("Received data at "));
      Serial.println(myData._millis);  
      Serial.print(F("Joystick : "));
      Serial.println(myData.joystick);
      Serial.print(F("Safety : "));
      Serial.println(myData.safety);
      Serial.print(F("Brake : "));
      Serial.println(myData.brake);
      Serial.print("Speed: ");
      Serial.println(throttle);
      Serial.println(F("************"));
    #endif
  }
  else if(timeOfLastMessage + TIMEOUT < millis()) {
    // shutdown motor
    if (timeout) {
      throttle -= ACCELERATION;
      if (throttle < MINSPEED) {
        throttle = MINSPEED;
      }
    }
    else {
      throttle = COASTSPEED;
      timeout = true;
    }
    
    ESC.write(throttle);
    
    // lightup LED signifying lost connection
    
    delay(250);
    #ifdef DEBUGMODE
      Serial.println(F("Remote Connection Lost"));
    #endif
  }
}
