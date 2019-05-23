#include <string.h>
void setup() {
  Serial.begin(9600);
}

long int num;


 void BinToDec(char bin[], int tam)
 {
    unsigned int i;
    num = 0;

   for(i=0; i < tam; i++)
   {
      if(bin[i] == '1')
      {
        num = ((num*2) +1);
      }
      if(bin[i] == '0')
      {
        num = (num*2);
      }
   }

 }


void loop() {
  //Serial.println(1650,HEX);
  //Serial.println(1650,BIN);
  //110 0111 0010
  //BinToDec("10101010101010101010101010101010‬", 32);
  //Serial.println(num, HEX);
String entrada = "0110011100100001000101010101010101010101010101010101010101010101010101010101010101000001000010100011011111111";
int strLenEntrada = entrada.length()-1;
unsigned char frame[strLenEntrada];
entrada.toCharArray(frame,strLenEntrada);

for(int i = 0; i< strLenEntrada;i++){
      Serial.print(frame[i]-48);
      
  }
  Serial.println("_____________________________");
  delay(2000);
}
