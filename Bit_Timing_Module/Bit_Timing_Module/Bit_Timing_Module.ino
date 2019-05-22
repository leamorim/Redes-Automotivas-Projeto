#include <TimerOne.h>

/******ENTRADAS DO MÓDULO*****/
#define TQ 1000000  //Tempo em Microssegundos

#define L_SYNC 1
#define L_PROP 1
#define L_PH_SEG1 6
#define L_PH_SEG2 8
/// Tamanhos de L_SEG1 E L_SEG2, saídas do módulo TQ_Configurator
#define L_SEG1 (L_PROP+L_PH_SEG1)
#define L_SEG2 L_PH_SEG2

#define SJW 10

enum estados {SYNC = 0,SEG1 = 1,SEG2 = 2} STATE_BT;

unsigned int count = 0;


bool Plot_Tq = false;
bool Sample_Point = false;
bool Writing_Point = false;
volatile bool Soft_Sync = false;
volatile bool Hard_Sync = false;
volatile bool SS_Flag = false;
volatile bool HS_Flag = false;
/*
void HS_ISR() {
  Hard_Sync = true;
  HS_Flag = true;
  Timer1.start();
  Timer1.attachInterrupt(Inc_Count,TQ);
 // Serial.println(Timer1.read());
}

void SS_ISR() {
  if(STATE != SYNC){
    Soft_Sync = true;
    SS_Flag = true;
  }  
}
*/

void Plotter(){// Função que adequa as variáveis para serem plotadas no serial plotter
  Serial.print(STATE_BT-1);
  Serial.print(",");
  Serial.print(Plot_Tq-3);
  Serial.print(",");
  Serial.print(HS_Flag-5);
  Serial.print(",");
  Serial.print(SS_Flag-7);
  Serial.print(",");
  Serial.print(Sample_Point-9);
  Serial.print(",");
  Serial.println(Writing_Point-11); 
}

void Inc_Count(){
  count++;
  Plot_Tq = !Plot_Tq; 
}

void print_state(){
  if(STATE_BT == SYNC){
    Serial.println("SYNC");
  }
  else if(STATE_BT == SEG1){
    Serial.println("SEG1");
  }
  else if(STATE_BT == SEG2){
    Serial.println("SEG2");
  }
}

int Ph_Error = 0;

void UC(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){
    Inc_Count();
    Serial.print("Count: ");
    Serial.println(count);
    print_state();
    
      switch(STATE_BT){
        case SYNC:
          if(count == L_SYNC){
              count = 0; //0 ou 1 ?
              STATE_BT = SEG1;
          }
          break;
        
        case SEG1:
          if(count == L_SEG1){
            STATE_BT = SEG2;
            count = 0;
          }
        
        break;
        
        case SEG2:
          if(count == L_SEG2){
            STATE_BT = SYNC;
            count = 0;
          }
        break;
  }
}



void setup() {
  Serial.begin(115200);
  Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(UC);
  STATE_BT = SYNC;
  count = 0;
  // Attach an interrupt to the ISR vector
 // attachInterrupt(0, HS_ISR, FALLING);
  //attachInterrupt(1, SS_ISR, FALLING);
}


void loop() {
  
  //Plotter();
}
