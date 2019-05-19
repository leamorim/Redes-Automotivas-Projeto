#include <string.h>
byte STATE;
unsigned int count = 0;
bool ERROR_FLAG = false;
bool BED_FLAG = false;
bool BUS_IDLE_FLAG = true;
unsigned int ACK_FLAG = false;
unsigned int SoF_FLAG = false;
unsigned int OVERLOAD_FLAG = false;


unsigned int aux_count = 0;
unsigned int Data_Flag = 0;
unsigned int Remote_Flag = 0;
unsigned int Extend_Flag = 0;

unsigned char Vetor_ID_A[11];
unsigned char Vetor_DLC[4];
unsigned char Vetor_ID_B[18];
unsigned char Vetor_DATA[64];
unsigned char Vetor_CRC[15];


unsigned int Value_ID_A;
unsigned int Value_DLC;
unsigned int Value_ID_B;
unsigned int Value_DATA;
unsigned int Value_CRC;
char DataHex[4];
int i;
long int num = 0;
bool ID_B_FLAG = true;

enum estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_ENC,STATE_DEC;
unsigned int count_encoder = 0;
unsigned int count_decoder = 0;
char last_bit_enc, last_bit_dec;

char BIT_TO_SAVE;
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
 #define FORMART_ERROR 26
 #define ACK_ERROR 27
 #define CRC_ERROR 28
 #define BIT_STUFFING_ERROR 29 
 #define STATE_BSD_FLAG1 30
 #define BIT_ERROR 31
 #define STATE_BED_FLAG1 32
 #define OVERLOAD 33


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

    switch (STATE_DEC){
    case INACTIVE:
        if(BSD_FLAG){
            STATE_DEC = COUNTING;
            count_decoder = 1;
        }
        else{
            STATE_DEC = INACTIVE;           
        }
        CAPTURE = true;
        BIT_TO_SAVE = Bit_Read;
        BSE_FLAG = false;
        last_bit_dec = Bit_Read;
        
        break;
        
    case COUNTING:
        if(!BSD_FLAG){
            STATE_DEC = INACTIVE;
            count_decoder = 0;
        }
        else{
            if(Bit_Read != last_bit_dec){
                count_decoder = 1;
            }
            else{
                count_decoder++;
                if(count_decoder == 5){//está no 5º bit lido
                  STATE_DEC = BIT_STUFFED;//estado q verifica o 6º bit lido
                } 
            }
        }

        BIT_TO_SAVE = Bit_Read;
        last_bit_dec = Bit_Read; 
        CAPTURE = true;
        BSE_FLAG = false;   
        
        break;    

        case BIT_STUFFED:
            count_decoder++;
            CAPTURE = false;            
            if(Bit_Read != last_bit_dec){//bit stuffing foi gerado corretamente e deve ser retirado     
                  count_decoder = 1;
                  if(BSD_FLAG){              
                      STATE_DEC = COUNTING;
                  }
              }
              else{//erro de bit stuffing
                  BSE_FLAG = true;
                  STATE_DEC = INACTIVE;
              }             
              last_bit_dec = Bit_Read;    
        break;
    }
  //Serial.println("Debugar:");
  //Serial.println(BIT_TO_SAVE);
  //Serial.println(Bit_Read);
  //Serial.println(CAPTURE);
}




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
    count++;
    switch(STATE)
    {
        case BUS_IDLE:
          if(BUS_IDLE_FLAG == true && count == 1)
          {
            Serial.println("Bus_idle");
            if(BIT_TO_SAVE == '0')
            {
              BUS_IDLE_FLAG = false;
              SoF_FLAG = true;
              count = 0;
              Serial.println("SoF");
              BSD_FLAG = true;
              STATE = ID_A;
            }
            else
            {
              count = 0;
              STATE = BUS_IDLE;
            }
          }
        break;


       /* case SoF:
         // if(count == L_BIT && frame[0] == '0' )
         // {
              Serial.println("SoF");
              BSD_FLAG = true;
              count  = 0;
              STATE = ID_A;
          //} 
        break;*/


        case ID_A:
          
          SoF_FLAG = false;
          Vetor_ID_A[count -1] = BIT_TO_SAVE;
          aux_count += 1;
          
          if(aux_count == 6)
          {
            aux_count = 0;            
            if(Vetor_ID_A[0] == '0' && Vetor_ID_A[1] == '0' && Vetor_ID_A[2] == '0' && Vetor_ID_A[3] == '0' && Vetor_ID_A[4] == '0' && Vetor_ID_A[0] == '0')
            {
              OVERLOAD_FLAG = true;
              count = 0;
              Serial.println("OVERLOAD FRAME"); 
              STATE = EoF;
            }
          }

          if(count == L_ID_A && BSD_FLAG == true && BED_FLAG == false)
          {
              
              BinToDec(Vetor_ID_A, 11);
              Value_ID_A = num;
              Serial.print("ID_A: 0x0");
              Serial.println(Value_ID_A,HEX);
              aux_count = 0;
              count  = 0;
              STATE = RTR_SRR;
          } 
        break;

        case RTR_SRR:
          if(count == L_BIT)
          {
              Serial.println("RTR_SRR");
              if(BIT_TO_SAVE == '0')
              {
                count  = 0;
                Data_Flag = 1; //Data Frame
                BED_FLAG = true;
                STATE = IDE_0; 
                Serial.println("RTR = 0");
                Serial.println("Data frame");
                // Base Data Frame or Format_Error
              }
              else
              {
                count  = 0;
                STATE = IDE_1;
                //Could be Base/extend Data/Remote frame
              }
          }
      break;

      case IDE_0:
          if(count == L_BIT && BED_FLAG == true)
          {
            Serial.println("IDE_0");
            Serial.println("IDE  = 0");
            if(BIT_TO_SAVE == '0')
            {
              Serial.println("Base frame Formart");
              count  = 0;
              STATE = R0;
            }
            else
            {
              Serial.println("Formart Error"); 
              count  = 0;
              STATE = FORMART_ERROR;
            }
          } 
        break;

        //States Extend

          case IDE_1:
            if(count == L_BIT)
            {
              Serial.println("IDE_1");
              if(BIT_TO_SAVE == '0')
              {
                Serial.println("RTR = 1");
                Serial.println("Remote Frame"); 
                Serial.println("IDE = 0");
                Serial.println("Base Frame Formart");
                        
                BED_FLAG = true;
                Remote_Flag = 1;
                count  = 0;
                STATE = R0;
              }
              else if(BIT_TO_SAVE == '1')
              {
                Serial.println("IDE = 1");
                Serial.println("Extend Frame");
                Extend_Flag = 1;
                count  = 0;
                STATE = ID_B;
              }  
            }
        break;

        case ID_B:
          
          Vetor_ID_B[count -1] = BIT_TO_SAVE;
          
          if(count == 1)
          {
            Serial.print("ID_B: 0x");
          }
          
          //Serial.print(BIT_TO_SAVE);
          DataHex[aux_count] = Vetor_ID_B[count -1];
          aux_count += 1;

          if(aux_count == 2 && ID_B_FLAG == true)
          {
            ID_B_FLAG = false;
            aux_count = 0;
            BinToHex('0','0',DataHex[0],DataHex[1]);
          }
          
          if(aux_count == 4 )
          {
            aux_count = 0;
            BinToHex(DataHex[0],DataHex[1],DataHex[2],DataHex[3]);
          }

          if(count == L_ID_B)
          {
            Serial.println("");
            //BinToDec(Vetor_ID_B, L_ID_B);
            //Value_ID_B = num;
            //Serial.print("ID_B: 0x0");
            //Serial.println(Value_ID_B,HEX);
            count  = 0;
            STATE = RTR;
          } 
        break;

        case RTR:
          if(count == L_BIT)
          {
            Serial.println("RTR");
            BED_FLAG = true;

            if(BIT_TO_SAVE == '0')
            {
              Serial.println("Data frame");
              Serial.println("RTR = 0"); 
              Data_Flag = 1;
              count  = 0;
              STATE = R1R0;  
            }
            else
            {
              Serial.println("Remote frame");
              Serial.println("RTR = 1");
              Remote_Flag = 1;
              count  = 0;
              STATE = R1R0;  
            }     
          } 
        break;

        case R1R0:

          if(count == L_R1R0 && BED_FLAG == true)
          {
            Serial.println("R1R0");
            count  = 0;
            STATE = DLC;
          }

        break;       


        //States Extend

        case R0:
          if(count == L_BIT)
          {
            
            Serial.println("R0");

            if((BIT_TO_SAVE == '0' || BIT_TO_SAVE == '1') && Remote_Flag == 0)
            {
              count  = 0;
              STATE = DLC;
            }
            else if((BIT_TO_SAVE == '0' || BIT_TO_SAVE == '1') && Remote_Flag == 1 && BED_FLAG == true )
            {
              count  = 0;
              STATE = DLC;

            }
        } 
        break;

        case DLC:
          
          Vetor_DLC[count - 1] = BIT_TO_SAVE;
          if(count == L_DLC)
          {               
            if(Data_Flag == 1 && Extend_Flag == 0)
            {         
              BinToDec(Vetor_DLC, 4);
              Value_DLC = (num > 8) ? 8 : num;  
              Serial.println("DLC");  
              Serial.print(Value_DLC);
              Serial.println("byte");
              
              if(Value_DLC == 0)
              {
                Serial.println("DATA: 0x00");
                count  = 0;          
                STATE = CRC_READ;
              } 
              else
              {
                count  = 0;          
                STATE = DATA;
              }
            }
            else if(Remote_Flag == 1 && Extend_Flag == 0)
            {
              Serial.println("DLC"); 
              Serial.println("DATA: 0x00");
              Remote_Flag = 0;
              count  = 0;
              STATE = CRC_READ;
            }
            else if(Data_Flag == 1 &&  Extend_Flag == 1)
            {        
              BinToDec(Vetor_DLC, 4);
              Value_DLC = (num > 8) ? 8 : num;
              Serial.println("DLC");  
              Serial.print(Value_DLC);
              Serial.println("byte");  

              if(Value_DLC == 0)
              {
                Serial.println("DATA: 0x00");
                count  = 0;          
                STATE = CRC_READ;
              } 
              else
              {
                count  = 0;          
                STATE = DATA;
              }

            }
            else if(Remote_Flag == 1 && Extend_Flag == 1)
            {

              Serial.println("DLC");
              Serial.println("DATA: 0x00");
              Remote_Flag = 0;
              Extend_Flag - 0;
              //BSD_FLAG = false;     
              count  = 0;
              STATE = CRC_READ;
            }
            
          } 
        break;

        case DATA:

          Vetor_DATA[count -1] = BIT_TO_SAVE;
          //Serial.print(Vetor_DATA[count -1] -48);
           if(count == 1)
          {
            Serial.print("DATA: 0x");
          }

          DataHex[aux_count] = Vetor_DATA[count -1];
          aux_count += 1;
          
          if(aux_count == 4 )
          {
            aux_count = 0;
            BinToHex(DataHex[0],DataHex[1],DataHex[2],DataHex[3]);
          }

          if(count == (Value_DLC*8))
          { 
            Serial.println("");

            if(Data_Flag == 1 && Extend_Flag == 0)
            {
              Data_Flag = 0;
              count  = 0;
              STATE = CRC_READ;
            }
            else if(Data_Flag == 1 && Extend_Flag == 1)
            {
              Data_Flag = 0;
              Extend_Flag = 0;
              count  = 0;
              STATE = CRC_READ;

            }
          } 
        break;

        case CRC_READ:
          
          Vetor_CRC[count -1] = BIT_TO_SAVE;
          Serial.print(BIT_TO_SAVE);
          aux_count = 0;
          
          if(count == L_CRC)
          {
              BinToDec(Vetor_CRC, L_CRC);
              Value_CRC = num;
              Serial.print("CRC: 0x0");
              Serial.println(Value_CRC,HEX);

              BSD_FLAG = false;
              count  = 0;
              STATE = CRC_DELIMITER;
          } 
        break;

        case CRC_DELIMITER:
          if(count == L_BIT && BSD_FLAG == false)
          {
              if(BIT_TO_SAVE == '1')
              {
                Serial.println("CRC_DELIMITER");
                count  = 0;
                STATE = ACK_SLOT;
              }
              else
              { 
                Serial.println("Formart Error");
                count  = 0;
                STATE = FORMART_ERROR;                
              }
          } 
        break;

        case ACK_SLOT:
          if(count == L_BIT)
          {
              if(BIT_TO_SAVE == '0')
              {
                Serial.println("ACK_SLOT");
                count  = 0;
                STATE = ACK_DELIMITER;
              }
              else
              {
                Serial.println("ACK ERROR");
                count  = 0;
                STATE = ACK_ERROR;             
              }
          } 
        break;

        case ACK_DELIMITER:
            if(count == L_BIT)
            {
                  //CRC Checked = CRC_READ
                  BSD_FLAG = false;
                  if(BIT_TO_SAVE == '1')
                  { 
                    Serial.println("ACK_DELIMITER");
                    count  = 0;
                    STATE = EoF;
                  }
                  else
                  {
                    Serial.println("FORMART ERROR");
                    count  = 0;
                    STATE = FORMART_ERROR;       
                  }                
                
            } 
        break;

        case EoF:
          //Serial.print(BIT_TO_SAVE);
        if(BIT_TO_SAVE == '1')
        {
          if(count == L_EOF)
          {
           
            Serial.println("EOF");
            count  = 0;          
            STATE = INTERFRAME_SPACING;
              
          }
        }
        else
        {
          count = 0;
          STATE = FORMART_ERROR;
        }
        
        break;

        case INTERFRAME_SPACING:
          Serial.print(BIT_TO_SAVE);
          
          if(BIT_TO_SAVE == '1')
            {
              //Serial.println(count);
              if(count == L_INTERFRAME_SPACING)
                  {
                      Serial.println("INTERFRAME_SPACING");
                      BUS_IDLE_FLAG  = true;
                      count  = 0;
                      STATE = BUS_IDLE;
                  }
            }
            else
            {
              Serial.println("INTERFRAME_SPACING");
              BUS_IDLE_FLAG = false;
              SoF_FLAG = true;
              count = 0;
              Serial.println("SoF");
              BSD_FLAG = true;
              BED_FLAG = false;
              STATE = ID_A;
            }
            
              
        break;
        
        //Error States

        case STATE_ERROR:
            Serial.println("STATE_ERROR");

            if(ERROR_FLAG)
            {
              STATE = STATE_ERROR;
            }
            else
            {
              STATE = BUS_IDLE;
            } 
        break;

        case FORMART_ERROR:
            
          ERROR_FLAG = true;
          Serial.println("FORMART_ERROR");
          STATE = STATE_ERROR;
              
        break;

        case ACK_ERROR:
          
          ERROR_FLAG = true;
          Serial.println("ACK_ERROR");  
          STATE = STATE_ERROR;
              
        break;

        case CRC_ERROR:

          ERROR_FLAG = true;
          Serial.println("CRC_ERROR");  
          STATE = STATE_ERROR;
              
        break;

        case OVERLOAD:

          OVERLOAD_FLAG = true;
          Serial.println("OVERLOAD");  
          STATE = STATE_ERROR;
              
        break;

        //Error States
    }
  }


  }
 
void setup() {
  Serial.begin(9600);
  STATE = BUS_IDLE;

}


void loop() {

String entrada = "011001110010000100010101010101010101010101010101010101010101010101010101010101010100000100001010001101111111110000000111111111110110011100100001000101010101010101010101010101010101010101010101010101010101010101000001000010100011011111111";
int strLenEntrada = entrada.length()+1;
unsigned char frame[strLenEntrada];
entrada.toCharArray(frame,strLenEntrada);

for(i = 0; i< strLenEntrada;i++){
  bit_stuffing_decoder(frame[i]);
  UC_DECODER();
  }

}
