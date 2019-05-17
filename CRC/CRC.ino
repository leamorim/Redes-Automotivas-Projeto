void setup(){
    Serial.begin(115200);
}

char *MakeCRC(char *BitString)
   {
   static char Res[16];                                 // CRC Result
   char CRC[15];
   int  i;
   char DoInvert;
   
   for (i=0; i<15; ++i)  CRC[i] = 0;                    // Init before calculation
   
   for (i=0; i<strlen(BitString); ++i)
      {
      DoInvert = ('1'==BitString[i]) ^ CRC[14];         // XOR required?

      CRC[14] = CRC[13] ^ DoInvert;
      CRC[13] = CRC[12];
      CRC[12] = CRC[11];
      CRC[11] = CRC[10];
      CRC[10] = CRC[9] ^ DoInvert;
      CRC[9] = CRC[8];
      CRC[8] = CRC[7] ^ DoInvert;
      CRC[7] = CRC[6] ^ DoInvert;
      CRC[6] = CRC[5];
      CRC[5] = CRC[4];
      CRC[4] = CRC[3] ^ DoInvert;
      CRC[3] = CRC[2] ^ DoInvert;
      CRC[2] = CRC[1];
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;
      }
      
   for (i=0; i<15; ++i)  Res[14-i] = CRC[i] ? '1' : '0'; // Convert binary to ASCII
   Res[15] = 0;                                         // Set string terminator

   return(Res);
   }

// A simple test driver:


void loop(){
  char *Data, *Result;
  Data = "‭01100111001000010001010101010101010101010101010101010101010101010101010101010101010‬";
  Result = MakeCRC(Data);                                    // Calculate CRC 
  Serial.print(Result);
  Serial.println("fim");
  delay(10000);
}
