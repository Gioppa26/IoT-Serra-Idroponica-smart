#ifndef _STATI_H_
#define _STATI_H_

#include <Arduino.h>

enum stati {st_level, st_fill, st_temp, st_ph, st_ec, st_adj};     //name different states

struct app_state {                          //data packet to pass and modify in the whole code
  enum stati current;
  bool first = true;
   float level;
   float T;
   float ph;
   float ecValue;
   float ECsetpoint=1.5;     //  desired EC and pH
   float pHsetpoint= 5.8;
   bool valveclosed=0;

};

void go(struct app_state *st, enum stati dest);         //go to another state function

void app_level(struct app_state *st);        //functions defined, code is in stati.cpp
void app_fill(struct app_state *st);
void app_temp(struct app_state *st);
void app_ph(struct app_state *st);
void app_ec(struct app_state *st);
void app_adj(struct app_state *st);


#endif
