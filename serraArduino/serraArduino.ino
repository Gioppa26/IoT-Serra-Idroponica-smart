#include "stati.h"

#define perpumpA 10  //peristaltic pump pins 
#define perpumpB 12
#define pHpumpC 11
#define valve 5      //electrovalve

app_state stato; //struct defined in stati.h

void setup() {
  Serial.begin(115200);            //inizializzazione della porta seriale principale
  pinMode(perpumpA, OUTPUT);        
  pinMode(perpumpB, OUTPUT);
  pinMode(pHpumpC, OUTPUT);
  pinMode(valve, OUTPUT);
  digitalWrite(valve, stato.valveclosed);
  delay(1000);
}

void loop() {
   static unsigned long timepoint = millis();
   // Intervallo di tempo per le funzioni degli stati (es. 1 secondo)
   if (millis() - timepoint > 1000U) {                  
     timepoint = millis();
     switch (stato.current) {              
        case st_level:
          app_level(&stato);
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
   
   sendToRaspberry();
}

//// FUNZIONI PER LA COMUNICAZIONE SERIALE CON RASPBERRY PI //////////

void sendToRaspberry() {
  static unsigned long lastSendTime = 0;
  // Invia i dati ogni 5 secondi (o altro intervallo desiderato)
  if (millis() - lastSendTime >= 5000) { 
    lastSendTime = millis();
    Serial.print("LEVEL:");
    Serial.println(stato.level, 2); //Livello dell'acqua
    Serial.print("Temperatura:");
    Serial.println(stato.T, 1); //Valore temperatura
    Serial.print("PH:");
    Serial.println(stato.ph, 2); //Valore PH
    Serial.print("EC:");
    Serial.println(stato.ecValue, 2); //Valore EC
  }
}