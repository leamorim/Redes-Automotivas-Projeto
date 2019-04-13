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

#define L_SEG1 4
#define L_SEG2 5
#define button1 2 //Porta digital 2 para o botão 1
#define button2 3 //Porta digital 3 para o botão 2

// variables will change:
//volatile int buttonStateHardSync = 0;         // variable for reading the pushbutton status
//volatile int buttonStateSoftSync = 0;         // variable for reading the pushbutton status

byte STATE;
bool Reset_Count = false;
unsigned int count = 0;

int HS = 0;       // Button 1 State - HARD SYNC
int soft = 0;       // Button 2 State - SOFT SYNC

void setup() {
  Serial.begin(115200);
  Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  Timer1.attachInterrupt(Inc_Count); //
  STATE = SYNC;
  pinMode(button1, INPUT);  // initialize the pushbutton 1 pin as an input:
  pinMode(button2, INPUT);  // initialize the pushbutton 2 pin as an input:
  // Attach an interrupt to the ISR vector
  attachInterrupt(0, HS_ISR, RISING);
  attachInterrupt(1, SS_ISR, RISING);
}

void HS_ISR() {
  //buttonStateHardSync = digitalRead(button1);//não precisa disso aqui
  Serial.println("HARD SYNC");
  STATE = SYNC;
  count = 0;
}

void SS_ISR() {
  //buttonStateSoftSync = digitalRead(button2);//não precisa disso aqui pq n uso essa var em nenhum outro lugar1
  Serial.println("SOFT SYNC");
}

void Inc_Count(){
  count++;
  Serial.print("STATE:");
  Serial.print(STATE);
  Serial.print("Count:");
  Serial.println(count);
}

void Plotter(){
   //Serial.print("STATE:");
  Serial.print(STATE-3);
  Serial.print(",");
  //Serial.print("Count:");
  Serial.println(count);
}

void UC(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){
  Plotter();
    switch(STATE){
      case SYNC:{
        if(count == L_SYNC){
          STATE = SEG1; 
          count = 0;
        }
        break;
      }
      case SEG1:{
        if(count == L_SEG1){
          STATE = SEG2;
          count = 0;
        }
        break;
      }
      case SEG2:{
        if(count == L_SEG2){
          STATE = SYNC;
          count = 0;
        }
        break;
      }
    }
}

void loop() {
  UC();
}
