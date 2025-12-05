#include "stati.h"
#include <Arduino.h>

#define valve 5      //electrovalve pin

#include <NewPing.h>
     
#define TRIGGER_PIN  9
#define ECHO_PIN     6
#define MAX_DISTANCE 1000
unsigned int distance;
    
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

#include <OneWire.h>                //libraries for DS18B20 temperature probe
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 7    //temp probe pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);     //initialize T probe

// PH sensor
#define PH_PIN A1    //pH pin

#define EC_PIN A2      // ec pin
#define VREF 5.0      //EC analog reference voltage(Volt) of the ADC
int rawValue = 0;
float voltage = 0.0;
float ecValue = 0.0;

#define tankH 17   //tank height in cm
#define tankL1 42    //l1 in cm
#define tankL2 35   //l2 in cm
#define totVol ((tankH-5)*tankL1*tankL2)/1000 //tank volume in L
#define concentration 100  //nutrient SOL A and B  concentration
#define correctionSpeed 70   //correction speed (%)
#define perpumpA 10  //peristaltic pump pins
#define perpumpB 12
#define pHpumpC 11
#define dosingUnit11 1.4996    // measured reach in ml/sec of 3 peristaltic pumps
#define dosingUnit13 1.5185
#define dosingUnit12 1.4826
#define pHoffset 0.2
#define ECoffset 0.05
int solAvol = (totVol*1000) / concentration;    //mL     for solution adjustment
int solBvol = (totVol*1000) / concentration;    //mL
int h2oVol = (totVol*1000) - (solAvol + solBvol);   //mL

void go(struct app_state *st, enum stati dest){    //function to change state, defined in stati.h
  st->current = dest;
  st->first = true;
}

void app_level(struct app_state *st){   //function to get tank level
  if (st->first) {
    st->first = false;
  }
  digitalWrite(valve, st->valveclosed);    //make sure valve is closed
  distance = (sonar.ping_cm());
  st->level = ((tankH - distance) * tankL1 * tankL2) / 1000;
  if(st->level < totVol ){   //check if level must be restored
    go(st, st_fill);         //if yes go to fill state
  } else {
    go(st, st_temp);
  }
}


void app_fill(struct app_state *st){
  if (st->first) {
    st->first = false;
  }

  while (st->level < totVol) {                        //open valve until level is restored
    digitalWrite(valve, !st->valveclosed);
    int lettura = sonar.ping_cm();
    st->level = ((tankH - lettura) * tankL1 * tankL2) / 1000;
   }
    go(st, st_level);
}



void app_temp(struct app_state *st){          //get temperature function
  if (st->first) {
    st->first = false;
  }
  sensors.begin();
  sensors.requestTemperatures();
  st->T = sensors.getTempCByIndex(0);
  go(st, st_ph);
}

void app_ph(struct app_state *st) {          
  if (st->first) {
    st->first = false;
  }
  float misura=0;
  misura = analogRead(PH_PIN);
  misura = misura / 10.0;
  float ph = (float)misura * (13.9/1024);
  ph = 1.7 * ph;
  st->ph = ph;
  //Serial.println(st->ph);
  go(st, st_ec);
}

void app_ec(struct app_state *st){        //get EC function
  if (st->first) {
    st->first = false;
  }
  rawValue = analogRead(EC_PIN);
  voltage = rawValue * ((float)VREF / 1023.0);   //read the analog value and store into the buffer
  (st->ecValue) = voltage * 1.33;
  go(st, st_adj);
}

void app_adj(struct app_state *st){              //adjust solution
  if (st->first) {
    st->first = false;
  }
  float deltaPH = st->ph - (st->pHsetpoint - pHoffset);           //calculate pH delta
  float deltaEC = (st->ECsetpoint - ECoffset) - st->ecValue;          //calculate EC delta
  //Serial.print("delta EC: ");
  //Serial.println(deltaEC, 2);
  //Serial.print("delta pH: ");
  //Serial.println(deltaPH);
  //   PROPORTION:  VTot=10L ; concentration 50:1   --->   solAvol / ECsetpoint = x / deltaEC;
if (deltaEC > ECoffset || deltaPH > pHoffset ) {                  //check if any correction is needed
    if (deltaEC > ECoffset){
    float  solA_qty = (deltaEC * solAvol) / (st->ECsetpoint);         //calculates SolA+B qty needed to adjust
    float  solB_qty = (deltaEC * solBvol) / (st->ECsetpoint);
    float dosingA = solA_qty * (1 / dosingUnit13);            //convert quantity in pumping time
    float dosingB = solB_qty * (1 / dosingUnit12);
    dosingA = (dosingA / 100) * correctionSpeed;        //adjust time for correction speed
    dosingB = (dosingB / 100) * correctionSpeed;
   /*
    Serial.print("EC set point:");            //debugging
    Serial.println( ECsetpoint - ECoffset);
    Serial.print("qtà sol A: ");
    Serial.println(solA_qty, 2);
    Serial.print("qtà sol B: ");
    Serial.println(solB_qty, 2);
    Serial.print("tempo dosaggio sol A: ");
    Serial.println(dosingA, 0);
    Serial.print("tempo dosaggio sol B: ");
    Serial.println(dosingB, 0);
    */
    digitalWrite(perpumpA, HIGH);               //activate pumps for the time needed
    //Serial.println("SolA dosing");
    delay(dosingA * 1000);
    delay(1000);
    digitalWrite(perpumpA, LOW);
    //Serial.println("SolA stop");
    delay(300);
    digitalWrite(perpumpB, HIGH);
    //Serial.println("SolB dosing");
    delay(dosingB * 1000);
    delay(1000);
    digitalWrite(perpumpB, LOW);
    //Serial.println("SolB stop");
    delay(300);
  }  if ( deltaPH > pHoffset ) {
    digitalWrite(pHpumpC, HIGH);
    //Serial.println("Acid dosing");          //activate acid pump for 1 sec
    delay(1000);                        //change this value empirically based on type of acid, total volume, correction time
    digitalWrite(pHpumpC, LOW);
    //Serial.println("Acid stop");
  }
  go(st, st_ph);
}
}