#include <stdlib.h>

#define CAN_RX_PIN 11
#define CAN_TX_PIN 10

char CAN_TX = '0';


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(10,OUTPUT);
  //if (Serial.available() > 0) {
 // CAN_TX = Serial.read();
//}
}

void loop() {
    if(CAN_TX == '0'){
      digitalWrite(10,HIGH);
      Serial.print("CAN TX HIGH - SENT: ");
    }
    else if(CAN_TX == '1'){
      digitalWrite(10,LOW);
      Serial.print("CAN TX LOW - SENT: ");
    }
    Serial.println(CAN_TX);
    delay(100);
}
