#include <TimerOne.h>

//******************ESTADOS DA UNIDADE DE CONTROLE*************//
#define SYNC 0
#define SEG1 1
#define SEG2 2

/***************************PARÂMETROS*************************/
#define TQ 1000000
#define L_PROP 2
#define L_PH_SEG1 3
#define L_PH_SEG2 7
#define L_SYNC 1
#define SJW 2
//tirar esse L_SEG1
#define L_SEG1 5
#define L_SEG2 7

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
bool reset_count = false;
bool reset_count_ok = true;

typedef struct{
  bool ss;
  bool hs;
}Edge_Detector;

//Variável Global
Edge_Detector Edge;

//***************************************************************//

void Inc_Count(){
  if(reset_count){
    count = 0;
    reset_count_ok = true;
    reset_count = false;
  }
  else{
    count++;
  }
  Serial.println("TQ");
  Serial.print("State:");
  Serial.print(STATE);
  Serial.print("/");
  Serial.print("CountInc: ");
  Serial.print(count);
  Serial.print("/");
  Serial.print(reset_count);
  Serial.print("/");
  Serial.println(reset_count_ok);
  Serial.print("hs/ss:");
  Serial.print(Edge.hs);
  Serial.print("/");
  Serial.println(Edge.ss);
  
}

void Edge_Detector_Module(/*CAN_RX,BUS_IDLE*/){
  //Simula um Hard_Sync ou Soft_Sync
  if(Serial.available() > 0){
    char c = Serial.read();
    if(c == 'h'){//ele entra nesse if apenas duas vezes, depois n entra mais, por isso ele só ativa hs e deixa pra UC desativar depois que realizar o hs
      Serial.println("Hard_Sync");
      Edge.hs = true; //faz essa atribuição duas vezes, verificar como serial com botão
      reset_count_ok = false;
    }
    else if(c == 's'){
      Serial.println("Soft_Sync");
      Edge.ss = true;
    }
  }
}

void UC(/*, SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){

  if(Edge.hs || !reset_count_ok){////A ideia de reset_count_ok é a sincronização já q o inc_count é chamado só de 1 em 1 TQ
    //Serial.print("STATE RESET: ");
    Edge.hs = false;
    reset_count = true;
    STATE = SYNC;
  }
  else{
        switch(STATE){
          case SYNC:{
            if(count == L_SYNC){
              STATE = SEG1;
              reset_count = true;        
            }
            else{
              STATE = SYNC; 
            }
            break;
          }
          case SEG1:{

            if(Edge.ss){
            //    Serial.println("SEG1 SOFT_SYNC");
                //end_seg1 = L_SEG1 + min(SJW,ph_error); //adicionar aqui o mínimo entre SJW e o phase_error
                int phase_error = min(count,SJW);
              //  Serial.print("phase_error:" );
              //  Serial.println(phase_error);
                
                if(count == L_SEG1 + phase_error){
                  reset_count = true;
                  Serial.print("Estado_Fim_Soft_SEG1: ");//Quando termina aqui ele não tá indo pra o estado SEG2
                  Edge.ss = false;
                  Serial.println(Edge.ss);
                  STATE = SEG2;
                }
            }
            else{
                if(count == L_SEG1){
                  STATE = SEG2;
                  reset_count = true;
                }
                else{
                  STATE = SEG1;
                }
            break;
            }
          case SEG2:{
            if(Edge.ss){//Falta colocar Soft_Sync no SEG2 tb, calcular o phase_error
              //int phase_error = 
              if(count >= L_SEG2/* - SJW*/){ //maior igual para o caso do count já ter passado do valor de L_SEG2-SJW
                  Edge.ss = false;
                  STATE = SEG2;
                  reset_count = true;
                }
            }
            else{
              if(count == L_SEG2){
                STATE = SYNC;
                reset_count = true;
              }
              else{
                STATE = SEG2;
              }
              break;
            }
          }
        }
    }
  }
}


void Bit_Timing_Module(){
  Edge_Detector_Module();
  UC();
}

void setup(){
  Serial.begin(115200);
  //TQ_Out = TQ_Configurator(); //Chama o TQ_Configurator //Faltou copiar TQ_Configurator pra cá, pra ficar mais simples, por enquanto, não coloquei ela aqui
  Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(Inc_Count);
  STATE = SYNC;//Inicialização da variável de estado
}

void loop(){
  Bit_Timing_Module();
  delay(500);
}
