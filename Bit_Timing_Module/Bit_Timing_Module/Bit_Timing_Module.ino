#include <TimerOne.h>

/******ENTRADAS DO MÓDULO*****/
#define TQ 1000000
#define L_PH_SEG1 3
#define L_PH_SEG2 5

#define L_PROP 1
#define L_SYNC 1
#define SYNC 0
#define SEG1 1
#define SEG2 2
#define SJW 3

#define L_SEG1 4
#define L_SEG2 5
#define button1 2 //Porta digital 2 para o botão 1
#define button2 3 //Porta digital 3 para o botão 2

byte STATE;
bool Reset_Count = false;
unsigned int count = 0;
bool Plot_Tq = false;

bool Sample_Point = false;
bool Writing_Point = false;
bool Soft_Sync = false;
bool Hard_Sync = false;

void setup() {
  Serial.begin(9600);
  Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(Inc_Count);
  STATE = SYNC;
  pinMode(button1, INPUT);  // initialize the pushbutton 1 pin as an input:
  pinMode(button2, INPUT);  // initialize the pushbutton 2 pin as an input:
  // Attach an interrupt to the ISR vector
  attachInterrupt(0, HS_ISR, RISING);
  attachInterrupt(1, SS_ISR, RISING);
}

void HS_ISR() {
  Hard_Sync = true;
}

void SS_ISR() {
  if(STATE != SYNC){
    Soft_Sync = true;
  }  
}

void Plotter(){

  Serial.print(STATE);
  Serial.print(",");
  Serial.print(Plot_Tq-2);
  Serial.print(",");
  Serial.print(Hard_Sync-4);
  Serial.print(",");
  Serial.print(Soft_Sync-6);
  Serial.print(",");
  Serial.print(Sample_Point-8);
  Serial.print(",");
  Serial.println(Writing_Point-10);

}

void Inc_Count(){
  count++;
  Plot_Tq = !Plot_Tq;
/*  Serial.print("STATE:");
  Serial.print(STATE);
  Serial.print("Count:");
  Serial.print(count);
  Serial.print("hs:");
  Serial.print(Hard_Sync);
  Serial.print("/ss: ");
  Serial.println(Soft_Sync);
  */
}

int Ph_Error = 0;

void UC(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){
    Writing_Point = false;
    Sample_Point = false;
    
    if(Hard_Sync){
//      Serial.println("HARD_SYNC");
      STATE = SYNC;
      count = 0;
      Ph_Error = 0;
      Hard_Sync = false;
      //FALTA RESETAR O TIMERONE AQUI
    }
    else{        
        switch(STATE){
        case SYNC:
          if(count == L_SYNC){
            STATE = SEG1; 
            count = 0;
            Ph_Error = 0;
          }
          break;
        
        case SEG1:{
            if(Soft_Sync){//Ver se é necessário resetar TimerOne aqui também
//              Serial.println("SOFT SYNC");
              int error = count;
              Ph_Error = min(SJW,error);
              Soft_Sync = false;  
//              Serial.print("error: ");
//              Serial.print(error);
//              Serial.print("/ Ph_error: ");
//              Serial.println(Ph_Error);
            }
            if(count == L_SEG1 + Ph_Error){
              STATE = SEG2;
              Sample_Point = true;
              count = 0;
              Ph_Error = 0;
            }
        break;
        }
        case SEG2:{
          if(Soft_Sync){
              int error = L_SEG2 - count + 1;
              Ph_Error = min(SJW,error);
              Soft_Sync = false;
//              Serial.print("error: ");
//              Serial.print(error);
//              Serial.print("/ Ph_error: ");
//              Serial.println(Ph_Error);
              if(count >= count - Ph_Error){
                STATE = SYNC;
                Writing_Point = true;
                Ph_Error = 0;
                count = 0;
              }
          }
          else if(count == L_SEG2 - Ph_Error){
            STATE = SYNC;
            Writing_Point = true;
            count = 0;
            Ph_Error = 0;
          }
          break;
      }
    }
   }
}


void loop() {
  
  Plotter();
  UC();

}
