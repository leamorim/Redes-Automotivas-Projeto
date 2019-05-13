byte STATE;
unsigned int count = 0;
unsigned int Value_DLC;
unsigned int ERROR_FLAG = 0;
unsigned int BSD_FLAG = 1;
unsigned int BED_FLAG = 0;
unsigned int RTR_FLAG;
unsigned int aux_cont = 0;
unsigned char frame[52] = {'0',
                           '0','0','0','0','0','0','1','0','1','0','0',
                           '0',
                           '0',
                           '0',
                           '0','0','0','1',
                           '0','0','0','0','0','0','0','1',
                           '0','1','0','0','0','0','1','1','0','0','0','0','0','0','0',
                           '1',
                           '0',
                           '1',
                           '1','1','1','1','1','1','1'
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
 #define EOF 12

/*Estados extras para o EXTEND*/
 #define IDE_1 13
 #define ID_B 14
 #define RTR 15
 #define R1R0 16 
 
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

 void CalculateDLC(char bit1, char bit2, char bit3, char bit4)
 {
   Value_DLC = ((bit1 - 48)*(8) + (bit2 - 48)*(4) + (bit3 - 48)*(2) + (bit4 - 48)*(1));
   if(Value_DLC > 8)
   {
    Value_DLC = 8; 
   }
 }


 void UC_DECODER(){

    switch(STATE)
    {
        case BUS_IDLE:
            if(frame[0] == '0')
            {
                Serial.println("Bus_idle");
                STATE = SoF;
            } 
        break;


        case SoF:
            if(count == L_BIT && BSD_FLAG == 1 && BED_FLAG == 0)
            {
                Serial.println("SoF");
                count  = 0;
                STATE = ID_A;
            } 
        break;


        case ID_A:
            if(count == L_ID_A)
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
                  RTR_FLAG = 0; //Data Frame
                  BED_FLAG = 1;
                  STATE = IDE_0; 
                  Serial.println("DATA FRAME");
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
                Serial.println("BASE FRAME FORMAT");
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

        //State Extend

         case IDE_1:
            if(count == L_BIT)
            {
              Serial.println("IDE_1");
              if(frame[13] == '0' && BED_FLAG == 1)
              {
                count  = 0;
                STATE = R0;
              }
              else if(frame[13] == '1')
              {
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
              count  = 0;
              STATE = R1R0;
            } 
        break;

        case R1R0:
            if(count == L_R1R0)
            {
              Serial.println("R1R0");
              count  = 0;
              STATE = DLC;
            } 
        break;       


        //State Extend

        case R0:
            if(count == L_BIT)
            {
                if(frame[14] == '0')
                {
                  Serial.println("R0");
                  count  = 0;
                  STATE = DLC;
                }
                else
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
                if(RTR_FLAG == 0)
                {
                  count  = 0;
                  CalculateDLC(frame[15], frame[16], frame[17], frame[18]);
                  Serial.print(Value_DLC);
                  Serial.println("byte");           
                  STATE = DATA;
                }
                else
                {
                  count  = 0;
                  STATE = CRC_READ;
                }
                //Faz a conta do DLC para saber o tamanho do campo data
            } 
        break;

        case DATA:
            if(count == L_DATA)
            {
                aux_cont = 19 + Value_DLC*8; //DATA começa na posição 19. 
                Serial.println("DATA");
                count  = 0;
                BSD_FLAG = 0;
                STATE = CRC_READ;
            } 
        break;

        case CRC_READ:
            if(count == L_CRC && BSD_FLAG == 0)
            {
                aux_cont += 15; 
                Serial.println("CRC_READ");
                count  = 0;
                STATE = CRC_DELIMITER;
            } 
        break;

        case CRC_DELIMITER:
            if(count == L_BIT)
            {
                if(frame[aux_cont] == '1')
                {
                  Serial.println("CRC_DELIMITER");
                  aux_cont++;
                  count  = 0;
                  STATE = ACK_SLOT;
                }
                else
                {
                  
                  Serial.println("Formart Error");
                  Serial.println(aux_cont);
                  Serial.println(frame[aux_cont]);
                  
                  count  = 0;
                  STATE = FORMART_ERROR;                
                }
            } 
        break;

        case ACK_SLOT:
            if(count == L_BIT)
            {
                if(frame[aux_cont] == '0')
                {
                  Serial.println("ACK_SLOT");
                  aux_cont++;
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
                 if(frame[aux_cont] == '1')
                 { 
                    Serial.println("ACK_DELIMITER");
                    count  = 0;
                    STATE = EOF;
                 }
                 else
                 {
                    Serial.println("FORMART ERROR");
                    count  = 0;
                    STATE = FORMART_ERROR;       
                 }                
                
            } 
        break;

        case EOF:
            if(count == L_EOF)
            {
                Serial.println("EOF");
                count  = 0;
                STATE = BUS_IDLE;
            } 
        break;


        //Error States

        case STATE_ERROR:
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
            
            STATE = STATE_ERROR;
             
        break;

        case ACK_ERROR:
            
            STATE = STATE_ERROR;
             
        break;

        case CRC_ERROR:
            
            STATE = STATE_ERROR;
             
        break;

        

        //Error States
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
