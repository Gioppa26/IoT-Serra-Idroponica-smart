#include "stati.h"

#define perpumpA 10  //peristaltic pump pins 
#define perpumpB 12
#define pHpumpC 11
#define valve 5      //electrovalve
#define LED_PIN 8

app_state stato; //struct defined in stati.h

void setup() {
  Serial.begin(115200);            //inizializzazione della porta seriale principale
  pinMode(perpumpA, OUTPUT);        
  pinMode(perpumpB, OUTPUT);
  pinMode(pHpumpC, OUTPUT);
  pinMode(valve, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(valve, stato.valveclosed);
  digitalWrite(LED_PIN, LOW);
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
   handlePiCommands();
   sendToRaspberry();
}

//// NUOVE FUNZIONI PER LA COMUNICAZIONE SERIALE CON RASPBERRY PI //////////

void handlePiCommands() {
  // Controlla se ci sono dati in arrivo sulla seriale dal Raspberry Pi
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n'); // Legge il comando fino a newline
    command.trim(); // Rimuove spazi bianchi

    if (command == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      //Serial.println("ACK: LED_ON"); // Feedback per il Pi
    } else if (command == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      //Serial.println("ACK: LED_OFF"); // Feedback per il Pi
    }
  }
}

//// FUNZIONI PER LA COMUNICAZIONE SERIALE CON RASPBERRY PI //////////

void sendToRaspberry() {
  static unsigned long lastSendTime = 0;
  // Invia i dati ogni 5 secondi (o altro intervallo desiderato)
  if (millis() - lastSendTime >= 5000) { 
    lastSendTime = millis();
    //Serial.println("Livello:");
    //Serial.println(stato.level,2);
    //Serial.print("Temperatura:");
    Serial.print(stato.T, 1); //Valore temperatura
    Serial.print(";");
    //Serial.print("PH:");
    Serial.print(stato.ph, 2); //Valore PH
    Serial.print(";");
    //Serial.print("EC:");
    Serial.print(stato.ecValue, 2); //Valore EC
    Serial.print(";");
    Serial.print(stato.level,2);
  }
}