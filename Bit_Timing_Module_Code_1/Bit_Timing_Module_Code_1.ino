#include <TimerOne.h>

//******************ESTADOS DA UNIDADE DE CONTROLE*************//
#define SYNC 1
#define SEG1 2
#define SEG2 3

/***************************PARÂMETROS*************************/
#define x 1000000 
#define L_PROP 2
#define L_PH_SEG1 3
#define L_PH_SEG2 7
#define L_SYNC 1
#define SJW 2

//************PONTOS A SEREM MOSTRADOS NA SAÍDA **************//
/*
 1. TQ
 2. Estados SYNC, SEG1, SEG2
 3. Hard_Sync (com um botão)
 4. Soft_Sync (com um botão)
 5. Sample Point
 6. Writing Point
*/

//******************VARIÁVEIS GLOBAIS**********************************//
byte count = 0; //contador
byte STATE; //estado

///VAI MORRER
bool hs = false;
bool ss = false;

//*************STRUCT DE SAÍDA DO MÓDULO DE CONFIGURAÇÃO DE TQ**********//
typedef struct  // Return struct of TQ_Configurator function
{
   byte L_SEG1; // Lenght of SEG1 in TQs;
   byte L_SEG2; // Lenght of SEG2 in TQs;
   unsigned long int TQ; // Time of TQ in microseconds*
}TQOut;  

typedef struct {
  bool HS;
  bool SS;
}ED_Out;

//ED_Out Edge;


TQOut TQ_Configurator() // Module of TQ Configuration
{
  unsigned int Bit_Length,L_SEG1;
  unsigned long int TQ_Time;
  TQOut TQ_Out;
  Bit_Length = L_SYNC + L_PROP + L_PH_SEG1 + L_PH_SEG2; //num de TQ's total de 1 bit
  TQ_Out.L_SEG1 = L_PROP + L_PH_SEG1; // Lenght of SEG1 = Length of PROPAGATION SEG + Length of PHASE SEG1
  TQ_Out.L_SEG2 = L_PH_SEG2; // Length of SEG2 = Length of PHASE SEG2
  TQ_Out.TQ = x;
  return TQ_Out;
}
TQOut TQ_Out;
ED_Out Edge;

void Edge_Detector(){/*Bus_Idle,Can_RX*/
  //Simula um Hard_Sync ou Soft_Sync
  if(Serial.available()){
    char c = Serial.read();
    if(c == 'h'){
      Serial.println("Hard_Sync");
      Edge.HS = true;
    }
    else if(c == 's'){
      Edge.SS = true;
      Serial.println("Soft_Sync");
    }
  }
}



void UC(/*Can_RX*/){
  
  if(Edge.HS){
    Edge.HS = false;
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
            if(Edge.SS){
                //end_seg1 = L_SEG1 + min(SJW,ph_error); //adicionar aqui o mínimo entre SJW e o phase_error
                int phase_error = min(count,SJW);
                Serial.print("phase_error:" );
                Serial.println(phase_error);
                
                if(count == (TQ_Out.L_SEG1 + phase_error)){
                  Serial.print("Estado_Fim_Soft: ");
                  Edge.SS = false;
                  STATE = SEG2;

                  Serial.println(STATE);
                  count = 0;
                }
                else{
                  Serial.println("ELSE EDGE");
                  Serial.println(count);
                  count++;
                }
            }
            else{
                if(count == TQ_Out.L_SEG1){
                  STATE = SEG2;
                  count = 0;
                  Serial.print("State:");
                  Serial.println(STATE);
                }
                else{
                  Serial.println("Else normal");
                  STATE = SEG1;
                  count++;
                }
            break;
          }
          case SEG2:{
            if(Edge.SS){
              if(count >= TQ_Out.L_SEG2/* - SJW*/){ //maior igual para o caso do count já ter passado do valor de L_SEG2-SJW
                  Edge.SS = false;
                  STATE = SEG2;
                  count = 0;
                }
                else
                  count++;
            }
            else{
              if(count == TQ_Out.L_SEG2){
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
    Serial.print("Count_value: ");
    Serial.println(count);
  }
      
}

void setup() {
  Serial.begin(115200);
  TQ_Out = TQ_Configurator(); //Chama o TQ_Configurator
  Timer1.initialize(TQ_Out.TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(UC);
  STATE = SYNC;//Inicialização da variável de estado
}

void Bit_Timing_Module(){
    Edge_Detector();
}

void loop() {
  Bit_Timing_Module();
}
