#include <stdlib.h>

#define CAN_RX_PIN 11
#define CAN_TX_PIN 10

int CAN_RX = 2;
char rcvd;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(11,INPUT);
}

void loop() {
    CAN_RX = digitalRead(11);
    if(CAN_RX == HIGH){
      Serial.println("CAN RX HIGH - Received");
      rcvd = '0';
    }
    else if(CAN_RX == LOW){
      Serial.println("CAN RX LOW - Received");
      rcvd = '1';
    }
    else{
      Serial.println("NOTHING ");
    }
    Serial.println(!CAN_RX);
    Serial.println(rcvd);
}
