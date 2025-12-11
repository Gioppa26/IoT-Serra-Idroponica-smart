#include "stati.h"
#include <Arduino.h>

#define valve 5      //pin valvola solenoide

//// LIBRERIE E SETUP SONAR ////
#include <NewPing.h>    

#define TRIGGER_PIN  9
#define ECHO_PIN     6
#define MAX_DISTANCE 200
unsigned int distance;
    
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

//// LIBRERIE E SETUP SENSORE TEMPERATURA(DS18B20) ////
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 7                   //pin sensore temperatura
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);     //inizializzazione del sensore

//// SETUP SENSORE PH ////
#define PH_PIN A1    //pin sensore PH
float misura=0;      //variabile per memorizzare il valore

//// SETUP SENSORE PH ////
#define EC_PIN A2      // pin sensore EC
#define VREF 5.0       // Tensione di riferimento (5V su Arduino UNO)
int rawValue = 0;
float voltage = 0.0;
float ecValue = 0.0;

#define tankH 17   //altezza vasca di contenimento
#define tankL1 42    //lunghezza vasca di contenimento
#define tankL2 35   //profondita vasca di contenimento
#define totVol ((tankH-10)*tankL1*tankL2)/1000 //volume massimo vasca di contenimento
#define concentration 100  //concentrazione nutriente SOL A e B
#define correctionSpeed 70   //velocita di correzione(%)
#define perpumpA 10  //pin pompe peristaltiche
#define perpumpB 12
#define pHpumpC 11
#define dosingUnit11 1.4996    //portata misurata in ml/sec delle 3 pompe peristaltiche
#define dosingUnit13 1.5185
#define dosingUnit12 1.4826
#define pHoffset 0.2 //offset dei valori PH e EC
#define ECoffset 0.05
int solAvol = (totVol*1000) / concentration;    //mL     variabile per la correzione dell'acqua con le soluzioni
int solBvol = (totVol*1000) / concentration;    //mL
int h2oVol = (totVol*1000) - (solAvol + solBvol);   //mL

//// FUNZIONE CAMBIAMENTO DI STATO, DEFINITA IN stati.h ////
void go(struct app_state *st, enum stati dest){
  st->current = dest;
  st->first = true;
}

//// FUNZIONE RILEVAZIONE LIVELLO ACQUA ////
void app_level(struct app_state *st){
  if (st->first) {
    st->first = false;
  }
  digitalWrite(valve, st->valveclosed);    //controlla che il solenoide sia chiuso
  distance = (sonar.ping_cm());
  st->level = ((tankH - distance) * tankL1 * tankL2) / 1000;
  if(st->level < totVol ){   //controlla che il livello massimo sia raggiunto
    go(st, st_fill);         //se e' sotto il livello continua a riempire la vasca
  } else {
    go(st, st_temp);
  }
}

//// FUNZIONE RIEMPIMENTO DELLA VASCA////
void app_fill(struct app_state *st){
  if (st->first) {
    st->first = false;
  }

  while (st->level < totVol) {                        //apre la valvola finche' il livello massimo non viene raggiunto
    digitalWrite(valve, !st->valveclosed);
    int lettura = sonar.ping_cm();
    st->level = ((tankH - lettura) * tankL1 * tankL2) / 1000;
   }
    go(st, st_level);
}

//// FUNZIONE RILEVAMENTO TEMPERATURA ////
void app_temp(struct app_state *st){
  if (st->first) {
    st->first = false;
  }
  sensors.begin();
  sensors.requestTemperatures();
  st->T = sensors.getTempCByIndex(0); // salva il valore nella variabile T definita in stati.h
  go(st, st_ph);
}

//// FUNZIONE RILEVAMENTO PH ////
void app_ph(struct app_state *st) {          
  if (st->first) {
    st->first = false;
  }
  misura = analogRead(PH_PIN);
  misura = misura / 10.0;
  float ph = (float)misura * (45.0/1024);
  ph = 1.7 * ph;
  st->ph = ph;
  go(st, st_ec);
}

//// FUNZIONE RILEVAMENTO EC ////
void app_ec(struct app_state *st){        //get EC function
  if (st->first) {
    st->first = false;
  }
  rawValue = analogRead(EC_PIN); 
  voltage = rawValue * ((float)VREF / 1023.0);   //legge il valore analogico e lo salva nel buffer
  (st->ecValue) = voltage * 1.33;
  go(st, st_adj);
}

//// FUNZIONE PER LE CORREZIONI ////
void app_adj(struct app_state *st){
  if (st->first) {
    st->first = false;
  }
  float deltaPH = st->ph - (st->pHsetpoint - pHoffset);           //calculate pH delta
  float deltaEC = (st->ECsetpoint - ECoffset) - st->ecValue;          //calculate EC delta
  
  //   PROPORZIONE:  VTot=10L ; concentrazione 50:1   --->   solAvol / ECsetpoint = x / deltaEC;
if (deltaEC > ECoffset || deltaPH > pHoffset ) {                  //controlla se ci sono correzioni necessarie
    if (deltaEC > ECoffset){ 
    float  solA_qty = (deltaEC * solAvol) / (st->ECsetpoint);   //calcola la quantita di SolA+SolB necessaria per la correzione
    float  solB_qty = (deltaEC * solBvol) / (st->ECsetpoint);
    float dosingA = solA_qty * (1 / dosingUnit13);            //converte la quantita in pumping time
    float dosingB = solB_qty * (1 / dosingUnit12);
    dosingA = (dosingA / 100) * correctionSpeed;        //regolazione del tempo di correzione
    dosingB = (dosingB / 100) * correctionSpeed;
    
    //Sezione di attivazione delle varie pompe in base al tempo necessario
    digitalWrite(perpumpA, HIGH);         
    delay(dosingA * 1000);
    delay(1000);
    digitalWrite(perpumpA, LOW);

    delay(300);
    digitalWrite(perpumpB, HIGH);
    delay(dosingB * 1000);
    delay(1000);
    digitalWrite(perpumpB, LOW);

    delay(300);
  }  if ( deltaPH > pHoffset ) {
    digitalWrite(pHpumpC, HIGH);
                                        //attuazione pompa con soluzione acida per 1s
    delay(1000);                        //modificare questo valore empiricamente in base al tipo di acido, al volume totale, al tempo di correzione
    digitalWrite(pHpumpC, LOW);
  }
  go(st, st_ph);
}
}