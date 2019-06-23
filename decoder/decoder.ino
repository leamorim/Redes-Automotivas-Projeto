#include <string.h>
#include <stdio.h>

byte STATE_DEC;
unsigned int count_decoder = 0;
bool ERROR_FLAG = false;
bool BED_FLAG = false;
bool BUS_IDLE_FLAG = true;
bool ACK_FLAG = false;
bool SoF_FLAG = false;
bool OVERLOAD_FLAG = false;
bool ID_B_FLAG = true;
bool CRC_FLAG = true;
unsigned int aux_count = 0;
unsigned int error_count = 0;
bool error_12 = false;
int count_6_12 = 0;
bool OVERLOAD_FLAG_1 = false;

unsigned int Data_Flag = 0;
unsigned int Remote_Flag = 0;
unsigned int Extended_Flag = 0;

char Vetor_RTR[1];
char Vetor_IDE[1];
char Vetor_ID_A[11];
char Vetor_DLC[4];
char Vetor_ID_B[18];
char Vetor_DATA[64];
char Vetor_CRC[15];
char *Resultado_CRC;
char Vetor_Frame[133];
int count_frame = 0;
char Vetor_Frame_aux[133];


long int Value_ID_A;
long int Value_DLC;
long int Value_ID_B;
long int Value_CRC;
char DataHex[4];
int i;


enum estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_BS_ENC,STATE_BS_DEC;
unsigned int count_bs_encoder = 0;
unsigned int count_bs_decoder = 0;
char last_bit_enc, last_bit_dec;

char BIT_TO_SAVE;
char BIT_TO_WRITE;
bool CAPTURE,BSE_FLAG, BSD_FLAG = true; 

 /*Estados*/
 #define BUS_IDLE 0
 #define SoF 1
 #define ID_A 2
 #define RTR_SRR 3
 #define IDE_0 4
 #define R0 5
 #define DLC 6
 #define DATA 7
 #define CRC_READ 8
 #define CRC_DELIMITER 9
 #define ACK_SLOT 10
 #define ACK_DELIMITER 11
 #define EoF 12
 #define INTERFRAME_SPACING 13

/*Estados extras para o EXTEND*/
 #define IDE_1 14
 #define ID_B 15
 #define RTR 16
 #define R1R0 17 
 
 /*Erro*/
 #define STATE_ERROR 25
 #define FORMAT_ERROR 26
 #define ACK_ERROR 27
 #define CRC_ERROR 28
 #define BIT_STUFFING_ERROR 29 
 #define STATE_BSD_FLAG1 30
 #define BIT_ERROR 31
 #define STATE_BED_FLAG1 32
 #define OVERLOAD 33
 #define WAIT 34


  /*Tamanho dos Estados*/
  #define L_BIT 1
  #define L_ID_A 11
  #define L_ID_B 18
  #define L_R1R0 2
  #define L_DLC 4
  #define L_DATA 8*Value_DLC
  #define L_CRC 15
  #define L_EOF 7
  #define L_INTERFRAME_SPACING 3

//Módulo de CRC:
//char *Data, *Result; //vars que foram usadas no teste
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
///FIM DO MÓDULO DE CRC

void bit_stuffing_decoder(char Bit_Read){
 /* Entradas:
    BSD_FLAG --> Indica se está num campo que pode ter bit stuffing ou não
    Bit_Read
    Sample_Point
    Saídas:
    BIT_TO_SAVE
    BSE_FLAG (Flag de Erro)
    CAPTURE (Sinal que manda o decoder capturar ou não o sinal)
 */   
//Assigns são feitos para estas saídas em TODOS os estados

    switch (STATE_BS_DEC){
    case INACTIVE:
        if(BSD_FLAG){
            STATE_BS_DEC = COUNTING;
            count_bs_decoder = 1;
        }
        else{
            STATE_BS_DEC = INACTIVE;           
        }
        CAPTURE = true;
        BIT_TO_SAVE = Bit_Read;
        BSE_FLAG = false;
        last_bit_dec = Bit_Read;
        
        break;
        
    case COUNTING:
        if(!BSD_FLAG){
            STATE_BS_DEC = INACTIVE;
            count_bs_decoder = 0;
        }
        else{
            if(Bit_Read != last_bit_dec){
                count_bs_decoder = 1;
            }
            else{
                count_bs_decoder++;
                if(count_bs_decoder == 5){//está no 5º bit lido
                  STATE_BS_DEC = BIT_STUFFED;//estado q verifica o 6º bit lido
                } 
            }
        }

        BIT_TO_SAVE = Bit_Read;
        last_bit_dec = Bit_Read; 
        CAPTURE = true;
        BSE_FLAG = false;   
        
        break;    

        case BIT_STUFFED:
            count_bs_decoder++;
            CAPTURE = false;            
            if(Bit_Read != last_bit_dec){//bit stuffing foi gerado corretamente e deve ser retirado     
                  count_bs_decoder = 1;
                  if(BSD_FLAG){              
                      STATE_BS_DEC = COUNTING;
                  }
              }
              else{//erro de bit stuffing
                  if(!OVERLOAD_FLAG_1)
                  {
                    Serial.println("BIT STUFFING ERROR");
                  }
                  BSE_FLAG = true;
                  STATE_BS_DEC = INACTIVE;
              }             
              last_bit_dec = Bit_Read;    
        break;
    }
}

/*
void frame_crc()
{
  //Serial.println("Vetor_frame");
  //Serial.println(Vetor_Frame);
  //Serial.println("");
  if(Extended_Flag == 0 && Data_Flag == 1 && Remote_Flag == 0)
  {
    Vetor_Frame_aux = (char*) calloc(19 + Value_DLC*8,sizeof(char));  //BASE DATA FRAME
    //Serial.print("Value_DLC*8:");
    //Serial.print(Value_DLC*8);
    //Serial.print("BASE DATA FRAME");
    for(i = 0; i < 19 + Value_DLC*8; i++)
    {
      Vetor_Frame_aux[i] = (char) Vetor_Frame[i];
      //Serial.print(Vetor_Frame_aux[i]);
    }
  }
  else if(Extended_Flag == 0 && Data_Flag == 0 && Remote_Flag == 1)
  {
    Vetor_Frame_aux = (char*) calloc(19,sizeof(char));  //BASE REMOTE FRAME
    //Serial.print("Caso 1");
    for(i = 0; i < 19; i++)
    {
      Vetor_Frame_aux[i] = (char) Vetor_Frame[i];
    }
  }
  else if(Extended_Flag == 1 && Data_Flag == 1 && Remote_Flag == 0)
  {
    //Serial.print("Caso 2");
    Vetor_Frame_aux = (char*) calloc(39 + Value_DLC*8,sizeof(char));  //EXTENDED DATA FRAME
    for(i = 0; i < 39 + Value_DLC*8; i++)
    {
      Vetor_Frame_aux[i] = (char) Vetor_Frame[i];
    }
  }
  else if(Extended_Flag == 1 && Data_Flag == 0 && Remote_Flag == 1)
  {
    //Serial.print("Caso 3");
    Vetor_Frame_aux = (char*) calloc(39,sizeof(char));  //EXTENDED REMOTE FRAME
    for(i = 0; i < 39; i++)
    {
      Vetor_Frame_aux[i] = (char) Vetor_Frame[i];
    }
  }

}*/

void check_id(unsigned int Extended_Flag,  char *ID_A_DECODER,  char *ID_B_DECODER,char *ID_A_FRAME,  char *ID_B_FRAME)
{
    bool ID_FLAG = true;
    int k = 0;

    for(k = 0; k < 11; k++)
    {
        if(ID_A_DECODER[k] != ID_A_FRAME[k])
        {
        ID_FLAG  = false;
        }

    }

    if(Extended_Flag)
    {
        for(k = 0; k < 18; k++)
        {
        if(ID_B_DECODER[k] != ID_B_FRAME[k])
        {
            ID_FLAG  = false;
        }

        }
    }

    if(ID_FLAG) 
    {
        Serial.println("ID Checked"); //Acender LED 
    }
    else
    {
        Serial.println("OUTRO ID"); //Acender outro LED
    }

}

void print_frame(char RTR_FRAME, char IDE_FRAME, char *ID_A_FRAME, char *ID_B_FRAME,unsigned int Value_DLC, char *DATA_FRAME, char *CRC_FRAME)
{
    long int Value;
    int i = 0;

    if(IDE_FRAME == '1')
    {
        Serial.println("SRR = 1");
        Serial.println("IDE = 1");
        Serial.println("EXTENDED FRAME");

        Value = BinToDec(ID_A_FRAME, 11);
        Serial.print("ID_A: 0x0");
        Serial.println(Value,HEX);

        Serial.print("ID_B: 0x0");
        BinToHex('0','0',ID_B_FRAME[0],ID_B_FRAME[1]);
        for(i = 0; i < 16; i += 4)
        {
          BinToHex(ID_B_FRAME[i+2],ID_B_FRAME[i+3],ID_B_FRAME[i+4],ID_B_FRAME[i+5]);
        }
        Serial.println("");
    }
    else if(IDE_FRAME == '0')
    {
        Serial.println("IDE = 0");
        Serial.println("BASE FRAME");
        Value = BinToDec(ID_A, 11);
        Serial.print("ID_A: 0x0");
        Serial.println(Value,HEX);
    }

    if(RTR_FRAME == '1')
    {
        Serial.println("RTR = 1");
        Serial.println("REMOTE FRAME");
        Serial.print("DLC: ");
        Serial.print(Value_DLC);

        Serial.println("");
        Serial.println("DATA: 0x00");
        
    }
    else if(RTR_FRAME == '0')
    {
        Serial.println("RTR = 0");
        Serial.println("DATA FRAME");
        Serial.print("DATA: 0x");

        for(i = 0; i < ((Value_DLC*8));  i += 4)
        {
          BinToHex(DATA_FRAME[i],DATA_FRAME[i+1],DATA_FRAME[i+2],DATA_FRAME[i+3]);
        }
        Serial.println("");
    }
    
    Value = BinToDec(CRC_FRAME, L_CRC);
    Serial.print("CRC:");
    Serial.println(CRC_FRAME); //HEX ???
}

 long int BinToDec(char bin[], int tam)
 {
    unsigned int i;
    long int num = 0;

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
  return num;
 }

 void BinToHex(char bit1, char bit2, char bit3, char bit4)
 {
    if(bit1 == '0' && bit2 == '0' && bit3 == '0' && bit4 == '0')
    {
      Serial.print("0");
    }
    else if (bit1 == '0' && bit2 == '0' && bit3 == '0' && bit4 == '1')
    {
      Serial.print("1");
    }
    else if (bit1 == '0' && bit2 == '0' && bit3 == '1' && bit4 == '0')
    {
      Serial.print("2");
    }
    else if (bit1 == '0' && bit2 == '0' && bit3 == '1' && bit4 == '1')
    {
      Serial.print("3");
    }
    else if (bit1 == '0' && bit2 == '1' && bit3 == '0' && bit4 == '0')
    {
      Serial.print("4");
    }
    else if (bit1 == '0' && bit2 == '1' && bit3 == '0' && bit4 == '1')
    {
      Serial.print("5");
    }
    else if (bit1 == '0' && bit2 == '1' && bit3 == '1' && bit4 == '0')
    {
      Serial.print("6");
    }
    else if (bit1 == '0' && bit2 == '1' && bit3 == '1' && bit4 == '1')
    {
      Serial.print("7");
    }
    else if (bit1 == '1' && bit2 == '0' && bit3 == '0' && bit4 == '0')
    {
      Serial.print("8");
    }
    else if (bit1 == '1' && bit2 == '0' && bit3 == '0' && bit4 == '1')
    {
      Serial.print("9");
    }
    else if (bit1 == '1' && bit2 == '0' && bit3 == '1' && bit4 == '0')
    {
      Serial.print("A");
    }
    else if (bit1 == '1' && bit2 == '0' && bit3 == '1' && bit4 == '1')
    {
      Serial.print("B");
    }
    else if (bit1 == '1' && bit2 == '1' && bit3 == '0' && bit4 == '0')
    {
      Serial.print("C");
    }
    else if (bit1 == '1' && bit2 == '1' && bit3 == '0' && bit4 == '1')
    {
      Serial.print("D");
    }
    else if (bit1 == '1' && bit2 == '1' && bit3 == '1' && bit4 == '0')
    {
      Serial.print("E");
    }
    else if (bit1 == '1' && bit2 == '1' && bit3 == '1' && bit4 == '1')
    {
      Serial.print("F");
    }
 }

void UC_DECODER()
{
  if(CAPTURE)
  {
    switch(STATE_DEC)
    {
      case BUS_IDLE:

        Serial.println("Bus_Idle");
        Vetor_Frame[0] = '0';
        Vetor_Frame_aux[0] = '0';
        count_frame = 1;
        
        if(OVERLOAD_FLAG_1)
        {
        Vetor_ID_A[count_decoder - 1] = BIT_TO_SAVE;
        //BSD_FLAG = false;
        STATE_DEC = ID_A;
        
        }
        else
        {
          if(BUS_IDLE_FLAG)
          {
            if(BIT_TO_SAVE == '0')
            {
                BUS_IDLE_FLAG = false;
                //SoF_FLAG = true;
                count_decoder = 0;
                Serial.println("SoF");
                BSD_FLAG = true;
                STATE_DEC = ID_A;
            }
            else if(BIT_TO_SAVE == '1')
            {
                Serial.println("Bus_Idle");
                BUS_IDLE_FLAG = true; 
                BSD_FLAG = false;               
                count_decoder = 0;
                STATE_DEC = BUS_IDLE;
            }
          }
        }
      break;

      case ID_A:
          
        //SoF_FLAG = false;
        //Serial.print(BIT_TO_SAVE);
        Vetor_ID_A[count_decoder - 1] = BIT_TO_SAVE;
        aux_count += 1;
        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
        Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
        
        
        if(OVERLOAD_FLAG_1 &&  aux_count == 4)
        {
        aux_count = 0;            
        if(Vetor_ID_A[1] == '0' && Vetor_ID_A[2] == '0' && Vetor_ID_A[3] == '0' && Vetor_ID_A[4] == '0')
        {
            OVERLOAD_FLAG = true;
            BSD_FLAG = false;
            error_12 = true;
            count_decoder = 0;
            Serial.println("OVERLOAD FRAME"); 
            STATE_DEC = STATE_ERROR;
        }
        else
        {
            OVERLOAD_FLAG_1 = false;
        }
        
        }


        if(count_decoder == L_ID_A && BSD_FLAG == true && BED_FLAG == false)
        {
            
            count_frame += 11;
            Value_ID_A = BinToDec(Vetor_ID_A, 11);
            Serial.print("ID_A: 0x0");
            Serial.println(Value_ID_A,HEX);
            aux_count = 0;
            count_decoder  = 0;
            STATE_DEC = RTR_SRR;
        } 
      break;

      case RTR_SRR:
          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          count_frame += 1;
          if(count_decoder == L_BIT)
          {
              //Serial.println("RTR_SRR");
              if(BIT_TO_SAVE == '0')
              {
              count_decoder  = 0;
              Data_Flag = 1; //Data Frame
              BED_FLAG = true;
              STATE_DEC = IDE_0; 
              //Serial.println("RTR = 0");
              Serial.println("Data frame");
              Vetor_RTR[0] = 0;
              // Base Data Frame or Format_Error
              }
              else if(BIT_TO_SAVE == '1')
              {
              count_decoder  = 0;
              STATE_DEC = IDE_1;
              //Could be Base/extend Data/Remote frame
              }
          }
      break;

      case IDE_0:
          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          count_frame += 1;
          if(count_decoder == L_BIT && BED_FLAG == true)
          {
          
          //Serial.println("IDE_0");
          if(BIT_TO_SAVE == '0')
          {
              //Serial.println("IDE  = 0");
              Serial.println("Base frame Format");
              Vetor_IDE[0] = 0;
              count_decoder  = 0;
              STATE_DEC = R0;
          }
          else
          {
              
              //Serial.println("IDE = 1");
              Serial.println("Format Error"); 
              count_decoder  = 0;
              STATE_DEC = FORMAT_ERROR;
          }
          } 
      break;

      //States Extend

      case IDE_1:
      Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
      Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
      count_frame += 1;
      if(count_decoder == L_BIT)
      {
          //Serial.println("IDE_1");
          if(BIT_TO_SAVE == '0')
          {
              //Serial.println("RTR = 1");
              Vetor_RTR[0] = 1;
              Serial.println("Remote Frame"); 
              //Serial.println("IDE = 0");
              Vetor_IDE[0] = 0;
              Serial.println("Base Frame Format");
              BED_FLAG = true;
              Remote_Flag = 1;
              count_decoder  = 0;
              STATE_DEC = R0;
          }
          else if(BIT_TO_SAVE == '1')
          {
              //Serial.println("SRR = 1");
              //Serial.println("IDE = 1");
              Vetor_IDE[0] = 1;
              Serial.println("Extend Frame");
              Extended_Flag = 1;
              count_decoder  = 0;
              STATE_DEC = ID_B;
          }  
      }
      break;

      case ID_B:
          
          Vetor_ID_B[count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          
          if(count_decoder == 1)
          {
          Serial.print("ID_B: 0x");
          }
          
          DataHex[aux_count] = Vetor_ID_B[count_decoder -1];
          aux_count += 1;

          if(count_decoder == 2 && ID_B_FLAG == true)
          {
            ID_B_FLAG = false;
            aux_count = 0;
            BinToHex('0','0',DataHex[0],DataHex[1]);
          }
          
          if(aux_count == 4 )
          {
            ID_B_FLAG == true;
            aux_count = 0;
            BinToHex(DataHex[0],DataHex[1],DataHex[2],DataHex[3]);
          }

          if(count_decoder == L_ID_B)
          {
            count_frame += 18;
            Serial.println("");
            count_decoder  = 0;
            STATE_DEC = RTR;
          } 
      break;

      case RTR:

          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_RTR[0] = BIT_TO_SAVE;
          count_frame += 1;

          if(count_decoder == L_BIT)
          {
            //Serial.println("RTR");
            BED_FLAG = true;

            if(BIT_TO_SAVE == '0')
            {
                Serial.println("Data frame");
                //Serial.println("RTR = 0"); 
                Data_Flag = 1;
                count_decoder  = 0;
                STATE_DEC = R1R0;  
            }
            else
            {
                Serial.println("Remote frame");
                //Serial.println("RTR = 1");
                Remote_Flag = 1;
                count_decoder  = 0;
                STATE_DEC = R1R0;  
            }     
          } 
      break;

      case R1R0:

          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;

          if(count_decoder == L_R1R0 && BED_FLAG == true)
          {
          count_frame += 2;
          //Serial.println("R1R0");
          count_decoder  = 0;
          STATE_DEC = DLC;
          }
      break;       

      //States Extend
      case R0:

          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          count_frame += 1;

          if(count_decoder == L_BIT)
          {
          
          //Serial.println("R0");

          if((BIT_TO_SAVE == '0' || BIT_TO_SAVE == '1') && Remote_Flag == 0)
          {
              count_decoder  = 0;
              STATE_DEC = DLC;
          }
          else if((BIT_TO_SAVE == '0' || BIT_TO_SAVE == '1') && Remote_Flag == 1 && BED_FLAG == true )
          {
              count_decoder  = 0;
              STATE_DEC = DLC;
          }
      } 
      break;

      case DLC:
          
        Vetor_DLC[count_decoder - 1] = BIT_TO_SAVE;
        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
        Vetor_Frame_aux[count_frame + count_decoder - 1] = BIT_TO_SAVE;

        if(count_decoder == L_DLC)
        { 
          count_frame += 4;
          Value_DLC = BinToDec(Vetor_DLC, 4);
          Value_DLC = (Value_DLC > 8) ? 8 : Value_DLC;  
          Serial.print("DLC: ");  
          Serial.print(Value_DLC);
          Serial.println("byte");

          if(Data_Flag == 1 && Extended_Flag == 0)
          {         
              
              if(Value_DLC == 0)
              {
                Serial.println("DATA: 0x00");
                count_decoder  = 0;          
                STATE_DEC = CRC_READ;
              } 
              else
              {
                count_decoder  = 0;          
                STATE_DEC = DATA;
              }
          }
          else if(Remote_Flag == 1 && Extended_Flag == 0)
          {
              //Remote_Flag = 0;
              count_decoder  = 0;
              STATE_DEC = CRC_READ;
          }
          else if(Data_Flag == 1 &&  Extended_Flag == 1)
          {        
              if(Value_DLC == 0)
              {
              Serial.println("DATA: 0x00");
              count_decoder  = 0;          
              STATE_DEC = CRC_READ;
              } 
              else
              {
                count_decoder  = 0;          
                STATE_DEC = DATA;
              }

          }
          else if(Remote_Flag == 1 && Extended_Flag == 1)
          {
              Serial.println("DATA: 0x00");
              //Remote_Flag = 0;
              //Extended_Flag - 0;  
              count_decoder  = 0;
              STATE_DEC = CRC_READ;
          }
        
          
        } 
      break;

      case DATA:

          Vetor_DATA[count_decoder -1] = BIT_TO_SAVE;
          //Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;

          if(count_decoder == 1)
          {
          Serial.print("DATA: 0x");
          }

          DataHex[aux_count] = Vetor_DATA[count_decoder -1];
          aux_count += 1;
          
          if(aux_count == 4 )
          {
            aux_count = 0;
            BinToHex(DataHex[0],DataHex[1],DataHex[2],DataHex[3]);
          }

          if(count_decoder == (Value_DLC*8))
          { 
            count_frame += (Value_DLC*8);
            Serial.println("");

          if(Data_Flag == 1 && Extended_Flag == 0)
          {
              //Data_Flag = 0;
              count_decoder  = 0;
              STATE_DEC = CRC_READ;
          }
          else if(Data_Flag == 1 && Extended_Flag == 1)
          {
              //Data_Flag = 0;
              //Extended_Flag = 0;
              count_decoder  = 0;
              STATE_DEC = CRC_READ;

          }
          } 
      break;

      case CRC_READ:
          
          Vetor_CRC[count_decoder -1] = BIT_TO_SAVE;
          Serial.print(BIT_TO_SAVE);
          aux_count = 0;

          if(count_decoder == L_CRC)
          {
            Value_CRC = BinToDec(Vetor_CRC, L_CRC);
            Serial.print("CRC: 0x0");
            Serial.println(Value_CRC,HEX);
            BSD_FLAG = false;
            count_decoder  = 0;
            STATE_DEC = CRC_DELIMITER;
          } 
      break;

      case CRC_DELIMITER:
        //frame_crc();
          if(count_decoder == L_BIT && BSD_FLAG == false)
          {
              if(BIT_TO_SAVE == '1')
              {
              Serial.println("CRC_DELIMITER");
              count_decoder  = 0;
              STATE_DEC = ACK_SLOT;
              }
              else
              { 
              Serial.println("CRC_DELIMITER Format Error");
              count_decoder  = 0;
              STATE_DEC = FORMAT_ERROR;                
              }
          } 
      break;

      case ACK_SLOT:
          if(count_decoder == L_BIT)
          {
                      
            /*if(BIT_TO_SAVE == '0')
            {
                Serial.println("ACK_SLOT");
                count_decoder  = 0;
                STATE_DEC = ACK_DELIMITER;
            }
            else
            {
                Serial.println("ACK ERROR");
                count_decoder  = 0;
                STATE_DEC = ACK_ERROR;             
            }*/
            Serial.println("ACK_SLOT");
            //BIT_TO_WRITE = '0';  
            ACK_FLAG = true;
            count_decoder  = 0;
            STATE_DEC = ACK_DELIMITER;
          } 
      break;

      case ACK_DELIMITER: 
          if(count_decoder == L_BIT)
          {
            //CRC Checked = CRC_READ
            //Concatenar campos
            if(Data_Flag == 1)
            {
              Resultado_CRC = MakeCRC(Vetor_Frame);
            }
            else if(Data_Flag == 0)
            {
              Resultado_CRC = MakeCRC(Vetor_Frame_aux);
            }
            
            /*Serial.println(" Resultado_CRC");
            for(int j = 0; j < 15; j++)
            {
                Serial.print(Resultado_CRC[j]);
            }
            Serial.println("Vetor_CRC ");
            for(int j = 0; j < 15; j++)
            {
                Serial.print(Vetor_CRC[j]);
            }
            */
            for(int j = 0; j < 15; j++)
            {
                //Serial.print(Resultado_CRC[j]);
                if(Resultado_CRC[j] != Vetor_CRC[j])
                {
                  CRC_FLAG = false;
                }

            }

            if(CRC_FLAG)
            {
                Serial.println("CRC Checked");
            }
            else
            {
                Serial.println("CRC ERROR");
                STATE_DEC = CRC_ERROR;
            }

            BSD_FLAG = false;
            if(BIT_TO_SAVE == '1')
            { 
                Serial.println("ACK_DELIMITER");
                count_decoder  = 0;
                STATE_DEC = EoF;
            }
            else if(BIT_TO_SAVE == '0')
            {
                Serial.println("ACK_DELIMITER FORMAT ERROR");
                count_decoder  = 0;
                STATE_DEC = FORMAT_ERROR;       
            }                
              
          }
          Data_Flag = 0;
          Extended_Flag = 0;
          Remote_Flag = 0; 
      break;

      case EoF:
          
      if(BIT_TO_SAVE == '1')
      {
          if(count_decoder == L_EOF)
          {
          
              Serial.println("EOF");
              count_decoder  = 0;          
              STATE_DEC = INTERFRAME_SPACING;
              
          }
      }
      else if(BIT_TO_SAVE == '0')
      {
        
        count_decoder = 0;
        STATE_DEC = FORMAT_ERROR;
      }
      
      break;

      case INTERFRAME_SPACING:
          //Serial.print(BIT_TO_SAVE);
          
          //print_frame(Vetor_RTR,Vetor_IDE,Vetor_ID_A,Vetor_ID_B,Value_DLC,Vetor_DATA,Vetor_CRC);
          //free(Vetor_Frame);
          //memset (Vetor_Frame,'-',133);
          
        if(BIT_TO_SAVE == '1')
        {
          if(count_decoder == L_INTERFRAME_SPACING)
              {
                  Serial.println("INTERFRAME_SPACING");
                  BUS_IDLE_FLAG  = true;
                  BED_FLAG = false;
                  count_decoder  = 0;
                  STATE_DEC = BUS_IDLE;
              }
        }
      else if(BIT_TO_SAVE == '0')
      {
          if(count_decoder > 1)
          {
            Serial.print("INTERFRAME_SPACING:");
            Serial.print(count_decoder-1);
            Serial.println("Bit(s)");
          }  
            OVERLOAD_FLAG_1 = true;
            BUS_IDLE_FLAG = true;
            //SoF_FLAG = true;
            count_decoder = 0;
            Serial.println("SoF");
            BSD_FLAG = true;
            BED_FLAG = false;
            STATE_DEC = BUS_IDLE;
      }       
      break;
      
      //Error States

      case STATE_ERROR:
          //Serial.println("STATE_ERROR");
          OVERLOAD_FLAG = false;
          OVERLOAD_FLAG_1 = false;
          //Serial.print(BIT_TO_SAVE);
          BSD_FLAG = false;
          error_count += 1;
          count_frame = 0;
          
          if(BIT_TO_SAVE == '0' && error_count <= 6)
          {
              //Serial.print("count_decoder: ");
              //Serial.println(count_decoder);
              if(count_decoder == 6)
              {
              error_count = 0;
              count_decoder = 0;
              count_6_12 += 1;
              error_12 = true;
              }
          }
          else if(BIT_TO_SAVE == '1' && error_count <= 8 && error_12 == true)
          {
              //Serial.print(BIT_TO_SAVE);
          
              if(count_decoder == 8)
              {
              error_12 = false;
              ERROR_FLAG = false;
              BED_FLAG = false;
              count_decoder = 0;
              error_count = 0;

              if(count_6_12 == 1)
              {
                  Serial.println("Error Frame (com 6 bits zeros no error flag)");
                  count_6_12 = 0;
              }
              else if(count_6_12 == 2)
              {
                  Serial.println("Error Frame (com 12 bits zeros no error flag)");
                  count_6_12 = 0;
              }
              if(ERROR_FLAG)
              {
                  STATE_DEC = STATE_ERROR;
              }
              else
              {
                  STATE_DEC = INTERFRAME_SPACING;
              }
              }
          }

      break;

      case FORMAT_ERROR:
          
          ERROR_FLAG = true;
          error_count = 0;
          Serial.println("FORMAT_ERROR");
          //Serial.print(BIT_TO_SAVE);
          STATE_DEC = STATE_ERROR;// come um bit
              
      break;

      case ACK_ERROR:
          
          ERROR_FLAG = true;
          error_count = 0;
          Serial.println("ACK_ERROR");  
          STATE_DEC = STATE_ERROR;
              
      break;

      case CRC_ERROR:

          ERROR_FLAG = true;
          error_count = 0;
          Serial.println("CRC_ERROR");  
          STATE_DEC = STATE_ERROR;
              
      break;

      case OVERLOAD:

          OVERLOAD_FLAG = true;
          error_count = 0;
          Serial.println("OVERLOAD");  
          STATE_DEC = STATE_ERROR;
              
      break;
      //Error States
      }
      count_decoder++;
  }
}


void setup() {
  Serial.begin(115200);
  STATE_DEC = BUS_IDLE;

}

void loop() {

String entrada = "011001110010000011111001010101010101010101010101010101010101010101010101010101011001100111011011111111010001001001111100000100000111110010000100010101010101010101010101010101010101010101010101010101010101010101111011111010101101111111101100111001000001001101010101100111000010101011111111";
int strLenEntrada = entrada.length()+1;
unsigned char frame[strLenEntrada];
entrada.toCharArray(frame,strLenEntrada);

for(i = 0; i< strLenEntrada;i++){
  bit_stuffing_decoder(frame[i]);
  UC_DECODER();
  }

if (i == strLenEntrada)
{
  while (1)
  {
    
  }
  
}


}