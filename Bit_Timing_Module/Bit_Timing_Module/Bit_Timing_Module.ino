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

#define SJW 4

enum estados {SYNC = 0,SEG1 = 1,SEG2 = 2} STATE_BT;

unsigned int count = 0;
int Ph_Error = 0;

volatile bool Plot_Tq = false;
volatile bool Sample_Point = false;
volatile bool Writing_Point = false;
volatile bool Soft_Sync = false;
volatile bool Hard_Sync = false;
volatile bool SS_Flag = false;

void SS_ISR() {
  if(STATE_BT == SEG1){
    Soft_Sync = true;
    Ph_Error = min(count,SJW);
    Serial.println("Soft_Sync SEG1");
    Serial.println(L_SEG1 + Ph_Error);
  }  
  else if(STATE_BT == SEG2){
    Ph_Error = min(((L_SEG2 + 1)-count),SJW);
    if(L_SEG2 - Ph_Error <= count){
      Serial.println("SS_flag, corte seg2");
      SS_Flag = true;
    }
    Soft_Sync = true;
    //Serial.println("Soft_Sync SEG2");
  }
}

void Plotter(){// Função que adequa as variáveis para serem plotadas no serial plotter
  Serial.print(STATE_BT+5);
  Serial.print(",");
  Serial.print(Plot_Tq-5);
  Serial.print(",");
  Serial.println(Hard_Sync-8);
  Serial.print(",");
  Serial.print(Soft_Sync-11);
  Serial.print(",");
  Serial.print(Sample_Point-13);
  Serial.print(",");
  Serial.println(Writing_Point-15); 
}

void Inc_Count(){
  count++;
  Plot_Tq = !Plot_Tq; 
}

void print_state(){
  Serial.print("Count: ");
  Serial.println(count);
  Serial.print("Ph_Error: ");
  Serial.println(Ph_Error);
  
  if(Hard_Sync){
    Serial.println("Hard_Sync");
  }
  if(Soft_Sync){
    Serial.println("Soft Sync");
    if(SS_Flag){
      Serial.println("corte SEG2");
    }
  }
 
  if(Sample_Point){
    Serial.println("Sample Point");
  }
  if(Writing_Point){
    Serial.println("Writing Point");
  }
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

void UC(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){
    Inc_Count();
    print_state();
    Writing_Point = false;
    Sample_Point = false;

      switch(STATE_BT){
        case SYNC:
          if(count == L_SYNC){
              count = 0; //0 ou 1 ?
              STATE_BT = SEG1;
          }
          break;
        
        case SEG1:
          if(count == (L_SEG1 +Ph_Error)){
            STATE_BT = SEG2;
            Sample_Point = true;
            count = 0;
            Ph_Error = 0;
          }
        
        break;
        
        case SEG2:
          if(count == (L_SEG2 - Ph_Error) || SS_Flag){
            if(SS_Flag){
              STATE_BT = SEG1;
            }
            else{
              STATE_BT = SYNC;
            }
            Writing_Point = true;
            SS_Flag = false;
            count = 0;
            Ph_Error = 0;
          }
        break;
  }
    Hard_Sync = false;
    Soft_Sync = false;
}



void HS_ISR() {
  Hard_Sync = true;
  Timer1.start(); //reinicia timerone
  Timer1.attachInterrupt(UC,TQ);//reinicia timerone
  count = 0;
  STATE_BT = SYNC;//talvez antes da reinicialização do timerone, verificar
  //Serial.println("Hard_Sync");
}



void setup() {
  Serial.begin(9600);
  Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(UC);
  STATE_BT = SYNC;
  count = 0;
  // Attach an interrupt to the ISR vector
  attachInterrupt(0, HS_ISR, FALLING);
  attachInterrupt(1, SS_ISR, FALLING);
}


void loop() {
  
  //Plotter();

}
