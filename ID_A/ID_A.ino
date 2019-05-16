#include<math.h>
void setup() {
  Serial.begin(9600);

}


 void BinToDec(char bin[], int tam)
 {
    unsigned int i;
    unsigned int num = 0;

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
  BinToDec("11001110010", 11);
  Serial.println(num, HEX);

}
