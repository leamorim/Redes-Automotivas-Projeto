#include <TimerOne.h>
#define TQ 1000000
#define SYNC 1
#define SEG1 2
#define SEG2 3
#define L_SEG1 3
#define L_SEG2 4
#define SJW 2

byte count = 0;
bool hs = false;
bool ss = false;
byte STATE;

void setup() {
  Serial.begin(115200);
  Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(UC);
  //TQ_Configurator(); //Chamar o TQConfigurator no setup
  STATE = SYNC;
}

void UC(){
  if(hs){
    hs = false;
    STATE = SYNC;
    count = 0;
  }
  else{
      switch(STATE){
        case SYNC:{
          if(count == 1){
            STATE = SEG1;
            count = 0;        
            Serial.print("State:");
            Serial.println(STATE);
          }
          else{
            count++;
            STATE = SYNC; 
          }
          break;
        }
        case SEG1:{
          if(ss){
              //end_seg1 = L_SEG1 + min(SJW,ph_error); //adicionar aqui o mínimo entre SJW e o phase_error
              if(count == L_SEG1 + SJW){
                ss = false;
                STATE = SEG2;
                count = 0;
              }
              else
                count++;
          }
          else{
              if(count == L_SEG1){
                STATE = SEG2;
                count = 0;
                Serial.print("State:");
                Serial.println(STATE);
              }
              else{
                STATE = SEG1;
                count++;
              }
          break;
        }
        case SEG2:{
          if(ss){
            if(count >= L_SEG2 - SJW){ //maior igual para o caso do count já ter passado do valor de L_SEG2-SJW
                ss = false;
                STATE = SEG2;
                count = 0;
              }
              else
                count++;
          }
          else{
            if(count == L_SEG2){
              STATE = SYNC;
              count = 0;
              Serial.print("State:");
              Serial.println(STATE);
            }
            else{
              STATE = SEG2;
              count++;
            }
            break;
          }
        }
      }
  }
  Serial.print("State:");
  Serial.println(STATE);
  Serial.print("Count_value: ");
  Serial.println(count);
}
}

void loop() {
//Soft_sync ou Hard_Sync
  if(Serial.available()){
    char c = Serial.read();
    if(c == 'h'){
      Serial.println("Hard_Sync");
      hs = true;
    }
    else if(c == 's'){
      ss = true;
      Serial.println("Soft_Sync");
    }
  }
}
