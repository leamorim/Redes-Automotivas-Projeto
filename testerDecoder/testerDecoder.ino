byte STATE;
unsigned int count = 0;
unsigned int ERROR_FLAG = 0;
unsigned int BSD_FLAG = 0;
unsigned int BED_FLAG = 0;
unsigned int BUS_IDLE_FLAG = 1;
unsigned int CAPTURE = 1;
unsigned int aux_count = 0;
unsigned int Data_Flag = 0;
unsigned int Remote_Flag = 0;
unsigned int Extend_Flag = 0;
unsigned int ACK_FLAG;
unsigned char BIT_TO_SAVE;
unsigned int SoF_FLAG = 0;

unsigned char Vetor_ID_A[11];
unsigned char Vetor_DLC[4];
unsigned char Vetor_ID_B[18];
unsigned char Vetor_DATA[56];
unsigned char Vetor_CRC[15];


unsigned int Value_ID_A;
unsigned int Value_DLC;
unsigned int Value_ID_B;
unsigned int Value_DATA;
unsigned int Value_CRC;

unsigned long int num = 0;

//Data Base Frame Format
unsigned char frame[112]= {'0',
                           '1','1','0','0','1','1','1','0','0','1','0',
                           '0',
                           '0',
                           '0',
                           '0','1','1','1',
                           '1','1','0','0','1','0','1','0','1','0','1','0','1','0','1','0','1','0','1','0',
                           '1','0','1','0','1','0','1','0','1','0','1','0','1','0','1','0','1','0','1','0',
                           '1','0','1','0','1','0','1','0','1','0','1','0','1','0','1','0',
                           '1','0','1','0','1','1','0','0','1','1','0','0','1','1','1',
                           '1',
                           '0',
                           '1',
                           '1','1','1','1','1','1','1',
                           '1','1','1'};


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


 void UC_DECODER(){

   if(CAPTURE == 1)
   {
      switch(STATE)
      {
          case BUS_IDLE:
            if(BUS_IDLE_FLAG == 1)
            {
              Serial.println("Bus_idle");
              if(frame[0] = '0')
              {
                BUS_IDLE_FLAG = 0;

                SoF_FLAG = 1;
                count = 0;
                STATE = SoF;
              }
              else
              {
                count = 0;
                STATE = BUS_IDLE;
              }
            }
          break;
  
  
          case SoF:
            if(count == L_BIT && frame[0] == '0' )
            {
                Serial.println("SoF");
                aux_count = 1;
                BSD_FLAG = 1;
                count  = 0;
                STATE = ID_A;
            } 
          break;
  
  
          case ID_A:
            
            SoF_FLAG = 0;
            Vetor_ID_A[count -1] = frame[aux_count + count - 1];

            if(count == L_ID_A && BSD_FLAG == 1 && BED_FLAG == 0)
            {
                
                BinToDec(Vetor_ID_A, 11);
                Value_ID_A = num;
                Serial.print("ID_A: 0x0");
                Serial.println(Value_ID_A,HEX);
                aux_count += 11;
                count  = 0;
                STATE = RTR_SRR;
            } 
          break;
  
          case RTR_SRR:
            if(count == L_BIT)
            {
                Serial.println("RTR_SRR");
                if(frame[aux_count] == '0')
                {
                  count  = 0;
                  Data_Flag = 1; //Data Frame
                  BED_FLAG = 1;
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
            aux_count += 1; 
        break;
  
        case IDE_0:
            if(count == L_BIT && BED_FLAG == 1)
            {
        
              Serial.println("IDE_0");
              Serial.println("IDE  = 0");
              if(frame[aux_count] == '0')
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
            aux_count += 1; 
          break;
  
          //States Extend
  
           case IDE_1:
              if(count == L_BIT)
              {
                Serial.println("IDE_1");
                if(frame[aux_count] == '0')
                {
                  Serial.println("RTR = 1");
                  Serial.println("Remote Frame"); 
                  Serial.println("IDE = 0");
                  Serial.println("Base Frame Formart");
                         
                  BED_FLAG = 1;
                  Remote_Flag = 1;
                  count  = 0;
                  STATE = R0;
                }
                else if(frame[aux_count] == '1')
                {
                  Serial.println("IDE = 1");
                  Serial.println("Extend Frame");
                  Extend_Flag = 1;
                  count  = 0;
                  STATE = ID_B;
                }  
              }
              aux_count += 1; 
          break;
  
          case ID_B:
            Vetor_ID_B[count -1] = frame[aux_count + count - 1];

            if(count == L_ID_B)
            {
              BinToDec(Vetor_ID_B, L_ID_B);
              Value_ID_B = num;
              Serial.print("ID_B: 0x0");
              Serial.println(Value_ID_B,HEX);
              aux_count += 18;
              count  = 0;
              STATE = RTR;
            } 
          break;
  
          case RTR:
            if(count == L_BIT)
            {
              Serial.println("RTR");
              BED_FLAG = 1;

              if(frame[aux_count] =='0')
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
              aux_count += 1;
            } 
          break;
  
          case R1R0:

            if(count == L_R1R0 && BED_FLAG == 1)
            {
              aux_count += 2;
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

              if(frame[aux_count] == '0' && Remote_Flag == 0)
              {
                count  = 0;
                STATE = DLC;
              }
              else if(frame[aux_count] == '0' && Remote_Flag == 1 && BED_FLAG == 1 )
              {
                count  = 0;
                STATE = DLC;

              }
              else if(frame[aux_count] =='1')
              {
                Serial.println("Formart Error");
                count  = 0;
                STATE = FORMART_ERROR;
              }
              aux_count += 1;
          } 
          break;
  
          case DLC:
            
            Vetor_DLC[count -1] = frame[aux_count + count - 1];
            //Serial.println(aux_count + count-1);
            if(count == L_DLC)
            {               
              aux_count += 4;
              if(Data_Flag == 1 && Extend_Flag == 0)
              {         
                BinToDec(Vetor_DLC, 4);
                Value_DLC = (num > 8) ? 8 : num;  
                Serial.println("DLC");  
                Serial.print(Value_DLC);
                Serial.println("byte"); 
                count  = 0;          
                STATE = DATA;
              }
              else if(Remote_Flag == 1 && Extend_Flag == 0)
              {

                Serial.println("DLC");   
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
                count  = 0;          
                STATE = DATA;
              }
              else if(Remote_Flag == 1 && Extend_Flag == 1)
              {

                Serial.println("DLC");
                Remote_Flag = 0;
                Extend_Flag - 0;   
                count  = 0;
                STATE = CRC_READ;
              }
              
            } 
          break;
  
          case DATA:
            
            
            Vetor_DATA[count -1] = frame[aux_count + count - 1];
            
            Serial.print(count -1);
            Serial.println(aux_count + count - 1);
            
            if(count == (Value_DLC*8))
            { 
              //Serial.println("aux_count");
              //Serial.println(aux_count);
              aux_count += Value_DLC*8;
              //Serial.println("aux_count");
              //Serial.println(aux_count);
              //Serial.println(Value_DLC*8);
              BinToDec(Vetor_DATA, (Value_DLC*8));
              Value_DATA = num;
              Serial.print("DATA: 0x0");
              Serial.println(Value_DATA,HEX);
              Serial.print("DATA: DEC");
              int i = 0;
              for(i = 0; i< (Value_DLC*8) ;i++)
              {
                Serial.print(Vetor_DATA[i]-48);  
              }
              
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
            
            Vetor_CRC[count -1] = frame[aux_count + count - 1];
            
            if(count == L_CRC)
            {
                //BinToDec(Vetor_CRC, L_CRC);
                //Value_CRC = num;
                Serial.print("CRC:");
                //Serial.println(Value_CRC,HEX);
                int i = 0;
                for(i = 0; i< 15;i++)
                {
                  Serial.print(Vetor_CRC[i]-48);  
                }

                BSD_FLAG = 0;
                aux_count += 15; 
                count  = 0;
                STATE = CRC_DELIMITER;
            } 
          break;
  
          case CRC_DELIMITER:
            if(count == L_BIT && BSD_FLAG == 0)
            {
                if(frame[aux_count] == '1')
                {
                  Serial.println("CRC_DELIMITER");
                  aux_count++;
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
                if(frame[aux_count] == '0')
                {
                  Serial.println("ACK_SLOT");
                  aux_count++;
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
                   if(frame[aux_count] == '1')
                   { 
                      Serial.println("ACK_DELIMITER");
                      aux_count++;
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
              if(frame[aux_count + count - 1] == '1')
              {
                //Serial.println(count);
                //Serial.println(frame[aux_count + count-1] - 48);    
                //Serial.println(aux_count+count-1);
                
                if(count == L_EOF)
                {
                    Serial.println("EOF");
                    aux_count += 7;
                    count  = 0;
                    STATE = INTERFRAME_SPACING;
                } 
              }
              else
              {
                count  = 0;
                STATE = FORMART_ERROR;
              }
              
          break;
  
          case INTERFRAME_SPACING:
            if(frame[aux_count + count - 1 ] == '1')
              {
                //Serial.println(count);
                //Serial.println(frame[aux_count + count -1] - 48);    
                //Serial.println(aux_count+count -1);
                if(count == L_INTERFRAME_SPACING)
                    {
                        Serial.println("INTERFRAME_SPACING");
                        aux_count+=3;
                        BUS_IDLE_FLAG  = 1;
                        count  = 0;
                        STATE = BUS_IDLE;
                    }
              }
              else
              {
                count  = 0;
                STATE = FORMART_ERROR;
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
              
            ERROR_FLAG = 1;
            Serial.println("FORMART_ERROR");
            STATE = STATE_ERROR;
               
          break;
  
          case ACK_ERROR:
            
            ERROR_FLAG = 1;
            Serial.println("ACK_ERROR");  
            STATE = STATE_ERROR;
               
          break;
  
          case CRC_ERROR:
  
            ERROR_FLAG = 1;
            Serial.println("CRC_ERROR");  
            STATE = STATE_ERROR;
               
          break;
  
          //Error States
      }
    }
  }
 
void setup() {
  Serial.begin(9600);
  STATE = BUS_IDLE;
  //entrada.toCharArray(frame,sizeof(frame));

}

void loop() {
  UC_DECODER();
  count++;

}
