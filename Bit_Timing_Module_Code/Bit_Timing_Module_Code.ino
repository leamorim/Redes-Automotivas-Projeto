#include <SPI.h>
#include "mcp_can.h"

/// Não precisa do F_osc e BRP
//***************************PARÂMETROS*************************//
#define TQ        200 //Tempo em milisegundos
#define L_PROP    1
#define L_PH_SEG1 3
#define L_PH_SEG2 3
#define L_SYNC    1


//************PONTOS A SEREM MOSTRADOS NA SAÍDA **************//
//*
 1. TQ
 2. Estado (SYNC, SEG1, SEG2)
 3. Hard_Sync (com um botão)
 4. Soft_Sync (com um botão)
*//


struct TQout // Return struct of TQ_Configurator function
{
   unsigned int L_SEG1; // Lenght of SEG1 in TQs;
   unsigned int L_PH_SEG2; // Lenght of SEG2 in TQs;
   float TQ_Time; // Time of TQ in miliseconds*
};  

// será q n seria interessante deixar esse função só no setup ?
struct TQout TQ_Configurator() // Module of TQ Configuration
{
  unsigned int TQ_Length,L_SEG1;
  float TQ_Time;
  struct TQout TQ_Out;
  TQ_Length = L_SYNC + L_PROP + L_PH_SEG1 + L_PH_SEG2;
  TQ_Out.L_SEG1 = L_PROP + L_PH_SEG1; // Lenght of SEG1 = Length of PROPAGATION SEG + Length of PHASE SEG1
  TQ_Out.L_PH_SEG2 = L_PH_SEG2; // Length of SEG2 = Length of PHASE SEG2
  TQ_Out.TQ_Time = TQ;
  return TQ_Out;
}

void UC(struct TQout){
  long int count;
  enum STATES {SYNC, SEG1, SEG2};
  
  switch (state){
    case SYNC:
    
    break;

    case SEG1:

    break;

    case SEG2:

    break;
  }

  
}

void BitTiming(){
  UC(TQ_configurator());
}

void setup() {
    Serial.begin(115200);
    /*
    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
*/
}

void loop() {
  BitTiming();
}
