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
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
}

void Inc_Count(){
  count++;
  Serial.print("STATE:");
  Serial.print(STATE);
  Serial.print("Count:");
  Serial.println(count);
}

void Edge_Detector(){
  HS = digitalRead(button1);
  soft = digitalRead(button2);
  if (HS == HIGH) {
    Serial.println("Hard_Sync");
  }
  if (soft == HIGH) {
    Serial.println("Soft_Sync");
  }
}


void UC(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){

  if(HS == HIGH){
    Serial.println("hard_entrou");
    HS = LOW;
    STATE = SYNC;
//    Timer1.start();//restart do contador
    count = 0;
  }
  else{
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
}

void loop() {
  Edge_Detector();
  UC();
  delay(100);
  // put your main code here, to run repeatedly:
}
