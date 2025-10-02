#include "stati.h"

#include "SerialTransfer.h"    //https://www.arduinolibraries.info/libraries/serial-transfer
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 4);  //initialize (rx,tx) the second serial port to communicate with esp8266
SerialTransfer myTransfer;          //initialize SerialTransfer protocol

const char* setpoint = "p";     //character sent as a signal to synchronize communication with esp8266
float newPHset;          //variables to store new setpoints coming from Blynk app
float newECset;

#define perpumpA 10  //peristaltic pump pins 
#define perpumpB 12
#define pHpumpC 11
#define valve 5      //electrovalve

app_state stato;     //struct defined in stati.h

void setup() {
 Serial.begin(115200);            //initialization of the serial ports
  mySerial.begin(57600);      
  myTransfer.begin(mySerial);   //SerialTransfer protocol on the second serial port

  pinMode(perpumpA, OUTPUT);        
  pinMode(perpumpB, OUTPUT);
  pinMode(pHpumpC, OUTPUT);
  pinMode(valve, OUTPUT);
  digitalWrite(valve, stato.valveclosed); //verify if it's closed on HIGH or LOW
  delay(1000);
}

void loop() {

   static unsigned long timepoint = millis();
  if (millis() - timepoint > 1000U) {                       //time interval: 1s= 1000U   
    timepoint = millis();
  switch (stato.current) {              //state's functions (app_xyz) defined in stati.cpp
    case st_level:
      app_level(&stato);    
      break;
    case st_fill:
      app_fill(&stato);
      break;    
    case st_temp:
      app_temp(&stato);
      break;   
    case st_ph:
      app_ph(&stato);
      break;   
    case st_ec:
      app_ec(&stato);
      break; 
    case st_adj: 
      app_adj(&stato);
      break; 
   }
  }
  blynkData();        //function to send values to blynkapp
  requestNewsetp () ;    //function to request for new setpoints 
}




////FUNCTIONS NEEDED FOR COMMUNICATION  WITH ESP8266 and SOLUTION ADJUSTMENT//////////

void blynkData() {                         //send data to blynk app, see SerialTransfer examples on Github
  while (!mySerial.available()) {}     //wait for signal to start communication
  char a = mySerial.read();       //read incoming singal
  if (a = "a") {               //verify signal
    uint16_t sendSize = 0;                                             //data conversion
    sendSize = myTransfer.txObj(stato.ecValue, sendSize);            //data from struct stato
    sendSize = myTransfer.txObj(stato.T, sendSize);
    sendSize = myTransfer.txObj(stato.ph, sendSize);
    sendSize = myTransfer.txObj(stato.level, sendSize);

    myTransfer.sendData(sendSize);             //send data
  }
}



void requestNewsetp () {
  mySerial.write(setpoint);            //send signal to start communication
  if (myTransfer.available()) {           //when the answer arrives
    uint16_t recSize = 0;   
    recSize = myTransfer.rxObj(newECset, recSize);        //convert data received
    recSize = myTransfer.rxObj(newPHset, recSize);
    if(newECset!=stato.ECsetpoint &&  newECset>0) { stato.ECsetpoint=newECset/10;}             //check if data received are different from current setpoints to change them
    if(newPHset!=stato.pHsetpoint  &&  newPHset>0) { stato.pHsetpoint=newPHset/10;}
    Serial.print("pH Setpoint: ");       //debugging
    Serial.println(stato.pHsetpoint);
     Serial.print("EC Setpoint: ");
    Serial.println(stato.ECsetpoint);   
  }
}
