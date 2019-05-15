byte STATE;
unsigned int count = 0;
unsigned int Value_DLC;
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

//Data Base Frame Format
unsigned char frame[112] = {'0',/*SoF*/
                           '0','0','0','0','0','0','1','0','1','0','0',/*ID_A*/
                           '0',/*RTR*/
                           '0',/*IDE*/
                           '0',/*R0*/
                           '0','0','0','1',/*DLC*/
                           '0','0','0','0','0','0','0','1',/*DATA*/
                           '0','1','0','0','0','0','1','1','0','0','0','0','0','0','0',/*CRC*/
                           '1',/*CRC_DELIMITER*/
                           '0',/*ACK*/
                           '1',/*ACK_DELIMITER*/
                           '1','1','1','1','1','1','1',/*EOF*/
                           '1','1','1'/*INTERFRAME*/
                           };

//Remote Base Frame Formart
unsigned char frame1[112] = {'0',/*SoF pos0*/
                           '0','0','0','0','0','0','1','0','1','0','0',/*ID_A pos 1-11*/
                           '1',/*RTR pos 12*/
                           '0',/*IDE pos 13*/
                           '0',/*R0 pos 14*/
                           '0','0','0','1',/*DLC pos 15-18*/
                           /*'0','0','0','0','0','0','0','1',/*DATA*/
                           '0','1','0','0','0','0','1','1','0','0','0','0','0','0','0',/*CRC*/
                           '1',/*CRC_DELIMITER*/
                           '0',/*ACK*/
                           '1',/*ACK_DELIMITER*/
                           '1','1','1','1','1','1','1',/*EOF*/
                           '1','1','1'/*INTERFRAME*/
                           };

//Data Extend Frame Formart
unsigned char frame2[132] = {'0',/*SoF pos 0*/
                           '0','0','0','0','0','0','1','0','1','0','0',/*ID_A pos 1-11*/
                           '1',/*SRR pos 12*/
                           '1',/*IDE pos 13*/
                           '0','0','0','0','0','0','1','0','1','0','0','0','0','0','0','0','0','1',/*ID_B pos 14-31*/
                           '0',/*RTR pos 32*/
                           '1','0',/*R1R0 pos 33-34*/
                           '0','0','0','1',/*DLC pos 35-38*/
                           '0','0','0','0','0','0','0','1',/*DATA pos começa em aux_count = 39 + Value_DLC*8 -1 39-46*/
                           '0','1','0','0','0','0','1','1','0','0','0','0','0','0','0',/*CRC pos aux_count+= 15*/
                           '1',/*CRC_DELIMITER pos aux_count+= 1*/
                           '0',/*ACK pos aux_count+= 1*/
                           '1',/*ACK_DELIMITER pos aux_count+= 1*/
                           '1','1','1','1','1','1','1',/*EOF pos aux_count+= 7*/
                           '1','1','1'/*INTERFRAME pos aux_count+= 3*/
                           };

//Remote Extend Frame Formart
unsigned char frame3[132] = {'0',/*SoF pos 0*/
                           '0','0','0','0','0','0','1','0','1','0','0',/*ID_A pos 1-11*/
                           '1',/*SRR pos 12*/
                           '1',/*IDE pos 13*/
                           '0','0','0','0','0','0','1','0','1','0','0','0','0','0','0','0','0','1',/*ID_B pos 14-31*/
                           '1',/*RTR pos 32*/
                           '1','0',/*R1R0 pos 33-34*/
                           '0','0','0','1',/*DLC pos 35-38*/
                           /*'0','0','0','0','0','0','0','1',/*DATA pos começa em aux_count = 39 + Value_DLC*8 -1 39-46*/
                           '0','1','0','0','0','0','1','1','0','0','0','0','0','0','0',/*CRC_READ pos aux_count+= 15 39-53*/
                           '1',/*CRC_DELIMITER pos aux_count+= 1 54*/
                           '0',/*ACK pos aux_count+= 1 55*/
                           '1',/*ACK_DELIMITER pos aux_count+= 1 56*/
                           '1','1','1','1','1','1','1',/*EOF pos aux_count+= 7 57-63*/
                           '1','1','1'/*INTERFRAME pos aux_count+= 3 64-66*/
                           };

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

 void number_of_bytes(char bit1, char bit2, char bit3, char bit4)
 {
   Value_DLC = ((bit1 - 48)*(8) + (bit2 - 48)*(4) + (bit3 - 48)*(2) + (bit4 - 48)*(1));
   if(Value_DLC > 8)
   {
    Value_DLC = 8; 
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
              if(frame[0] = '0')
              {
                BUS_IDLE_FLAG = 0;
                Serial.println("Bus_idle");
                count = 0;
                STATE = SoF;
              }
              else
              {
                Serial.println("Bus_idle");
                count = 0;
                STATE = BUS_IDLE;
              }
            }
          break;
  
  
          case SoF:
            if(count == L_BIT && frame[0] == '0' )
            {
                Serial.println("SoF");
                BSD_FLAG = 1;
                count  = 0;
                STATE = ID_A;
            } 
          break;
  
  
          case ID_A:
            if(count == L_ID_A && BSD_FLAG == 1 && BED_FLAG == 0)
            {
                Serial.println("ID_A");
                count  = 0;
                STATE = RTR_SRR;
            } 
          break;
  
          case RTR_SRR:
            if(count == L_BIT)
            {
                Serial.println("RTR_SRR");
                if(frame[12] == '0')
                {
                  count  = 0;
                  Data_Flag = 1; //Data Frame
                  BED_FLAG = 1;
                  STATE = IDE_0; 
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
            if(count == L_BIT && BED_FLAG == 1)
            {
              Serial.println("IDE_0");
              if(frame[13] == '0')
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
                if(frame[13] == '0')
                {
                  Serial.println("Base Frame Formart");
                  Serial.println("Remote Frame");        
                  BED_FLAG = 1;
                  Remote_Flag = 1;
                  count  = 0;
                  STATE = R0;
                }
                else if(frame[13] == '1')
                {
                  Serial.println("Extend Frame");
                  Extend_Flag = 1;
                  count  = 0;
                  STATE = ID_B;
                }  
              } 
          break;
  
          case ID_B:
            if(count == L_ID_B)
            {
              Serial.println("ID_B");
              count  = 0;
              STATE = RTR;
            } 
          break;
  
          case RTR:
            if(count == L_BIT)
            {
              Serial.println("RTR");
              BED_FLAG = 1;
              
              if(frame[32] =='0')
              {
                Serial.println("Data frame");
                Data_Flag = 1;
                count  = 0;
                STATE = R1R0;  
              }
              else
              {
                Serial.println("Remote frame");
                Remote_Flag = 1;
                count  = 0;
                STATE = R1R0;  
              }     
              
            } 
          break;
  
          case R1R0:
            if(count == L_R1R0 && BED_FLAG == 1)
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
                if(frame[14] == '0' && Remote_Flag == 0)
                {
                  Serial.println("R0");
                  count  = 0;
                  STATE = DLC;
                }
                else if(frame[14] == '0' && Remote_Flag == 1 && BED_FLAG == 1 )
                {
                  Serial.println("R0");
                  count  = 0;
                  STATE = DLC;
  
                }
                else if(frame[14] =='1')
                {
                  Serial.println("Formart Error");
                  count  = 0;
                  STATE = FORMART_ERROR;
                }
            } 
          break;
  
          case DLC:
            if(count == L_DLC)
            {
                if(Data_Flag == 1 && Extend_Flag == 0)
                {
                  number_of_bytes(frame[15], frame[16], frame[17], frame[18]);
                  Serial.println("DLC");  
                  Serial.print(Value_DLC);
                  Serial.println("byte"); 
                  count  = 0;          
                  STATE = DATA;
                }
                else if(Remote_Flag == 1 && Extend_Flag == 0)
                {
  
                  Serial.println("DLC");   
                  aux_count = 19;
                  Remote_Flag = 0;
                  count  = 0;
                  STATE = CRC_READ;
                }
                else if(Data_Flag == 1 &&  Extend_Flag == 1)
                {
                  aux_count = 35;
                  number_of_bytes(frame[aux_count], frame[aux_count+1], frame[aux_count+2], frame[aux_count+3]);
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
                  aux_count = 39;
                  count  = 0;
                  STATE = CRC_READ;
                }
              
            } 
          break;
  
          case DATA:
            if(count == L_DATA)
            {
                if(Data_Flag == 1 && Extend_Flag == 0)
                {
                  aux_count = 19 + Value_DLC*8; //DATA começa na posição 19. 
                  Serial.println("DATA");
                  Data_Flag = 0;
                  count  = 0;
                  STATE = CRC_READ;
                }
                else if(Data_Flag == 1 && Extend_Flag == 1)
                {
                  aux_count = 39 + Value_DLC*8;//DATA começa na posição 39.
                    Serial.println("DATA");
                    Data_Flag = 0;
                    Extend_Flag = 0;
                    count  = 0;
                    STATE = CRC_READ;
  
                }
            } 
          break;
  
          case CRC_READ:
            if(count == L_CRC)
            {
                BSD_FLAG = 0;
                aux_count += 15; 
                Serial.println("CRC_READ");
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
                  Serial.println(aux_count);
                  Serial.println(frame[aux_count]);
                  
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
                  Serial.println("ACK Error");
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

}

void loop() {
  UC_DECODER();
  count++;

}
