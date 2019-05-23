//Bibliotecas e Defines BEGIN
#include <TimerOne.h>
    //Bit_Timing Defines
#define TQ 1000000  //Tempo em Microssegundos
#define L_SYNC 1
#define L_PROP 1
#define L_PH_SEG1 6
#define L_PH_SEG2 8
#define SJW 4
/// Tamanhos de L_SEG1 E L_SEG2, saídas do módulo TQ_Configurator
#define L_SEG1 (L_PROP+L_PH_SEG1)
#define L_SEG2 L_PH_SEG2


//Bibliotecas e Defines END



//Variáveis Globais BEGIN
    //Bit_Timing Variáveis
enum estados {SYNC = 0,SEG1 = 1,SEG2 = 2} STATE_BT;
unsigned int count_bt = 0;
int Ph_Error = 0;
volatile bool Plot_Tq = false;
volatile bool Sample_Point = false;
volatile bool Writing_Point = false;
volatile bool Soft_Sync = false;
volatile bool Hard_Sync = false;
volatile bool SS_Flag = false;


//Variáveis Globais END



//Bit_Timing_Module BEGIN
void SS_ISR() {
  if(STATE_BT == SEG1){
    Soft_Sync = true;
    Ph_Error = min(count_bt,SJW);
  }  
  else if(STATE_BT == SEG2){
    Ph_Error = min(((L_SEG2 + 1)-count_bt),SJW);
    if(L_SEG2 - Ph_Error <= count_bt){
      SS_Flag = true;
    }
    Soft_Sync = true;
  }
}

void Inc_Count(){
  count_bt++;
  Plot_Tq = !Plot_Tq; 
}

void UC(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){
    Inc_Count();
    print_state();
    Writing_Point = false;
    Sample_Point = false;

      switch(STATE_BT){
        case SYNC:
          if(count_bt == L_SYNC){
              count_bt = 0; //0 ou 1 ?
              STATE_BT = SEG1;
          }
          break;
        
        case SEG1:
          if(count_bt == (L_SEG1 +Ph_Error)){
            STATE_BT = SEG2;
            Sample_Point = true;
            count_bt = 0;
            Ph_Error = 0;
          }
        
        break;
        
        case SEG2:
          if(count_bt == (L_SEG2 - Ph_Error) || SS_Flag){
            if(SS_Flag){
              STATE_BT = SEG1;
            }
            else{
              STATE_BT = SYNC;
            }
            Writing_Point = true;
            SS_Flag = false;
            count_bt = 0;
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
  count_bt = 0;
  STATE_BT = SYNC;//talvez antes da reinicialização do timerone, verificar
}


//Bit_Timing_Module END



//Encoder BEGIN



//Encoder END



//Decoder BEGIN



//Decoder END



//Setup BEGIN

void setup() {
  Serial.begin(9600);
  Timer1.initialize(TQ);
  Timer1.attachInterrupt(UC);
  STATE_BT = SYNC;
  count_bt = 0;
  attachInterrupt(0, HS_ISR, FALLING);
  attachInterrupt(1, SS_ISR, FALLING);
}



//Setup END


//Loop BEGIN

void loop(){

}

//Loop END