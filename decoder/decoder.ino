byte STATE;
unsigned int count = 0;
unsigned int Value_DLC;
unsigned int ERROR_FLAG = 0;
unsigned int BSD_FLAG = 1;
unsigned int BED_FLAG = 0;

 /*Estados*/
 #define BUS_IDLE 0
 #define SoF 1
 #define ID_A 2
 #define RTR_SRR 3
 #define IDE 4
 #define R0 5
 #define DLC 6
 #define DATA 7
 #define CRC_READ 8
 #define CRC_DELIMITER 9
 #define ACK_SLOT 10
 #define ACK_DELIMITER 11
 #define EOF 12

/*Estados extras para o EXTEND*/
 #define ID_B 13
 #define RTR 14
 #define R1R0 15 
 
 /*Erro*/
 #define STATE_ERROR 16
 #define FORMART_ERROR 17
 #define ACK_ERROR 18
 #define CRC_ERROR 19
 #define BIT_STUFFING_ERROR 20 
 #define STATE_BSD_FLAG1 21
 #define BIT_ERROR 22
 #define STATE_BED_FLAG1 23

 
  /*Tamanho dos Estados*/
  #define L_BIT 1
  #define L_ID_A 11
  #define L_ID_B 18
  #define L_R1R0 2
  #define L_DLC 4
  #define L_DATA 8*Value_DLC
  #define L_CRC 15
  #define L_EOF 7


 void UC_DECODER(){

    switch(STATE)
    {
        case BUS_IDLE:
            if(count == L_BIT)
            {
                STATE = SoF;
            } 
        break;


        case SoF:
            if(count == L_BIT && BSD_FLAG == 1 && BED_FLAG == 0)
            {
                STATE = ID_A;
            } 
        break;


        case ID_A:
            if(count == L_ID_A)
            {
                STATE = RTR_SRR;
            } 
        break;

        case RTR_SRR:
            if(count == L_BIT)
            {
                STATE = IDE;
            } 
        break;

        case IDE:
            if(count == L_BIT)
            {
             STATE = R0;
            } 
        break;

        //State Extend
        case ID_B:
            if(count == L_ID_B)
            {
             STATE = RTR;
            } 
        break;

        case RTR:
            if(count == L_BIT)
            {
             STATE = R1R0;
            } 
        break;

        case R1R0:
            if(count == L_R1R0)
            {
             STATE = DLC;
            } 
        break;       


        //State Extend

        case R0:
            if(count == L_BIT)
            {
                STATE = DLC;
            } 
        break;

        case DLC:
            if(count == L_DLC)
            {
                STATE = DATA;
            } 
        break;

        case DATA:
            if(count == L_DATA)
            {
                STATE = CRC_READ;
            } 
        break;

        case CRC_READ:
            if(count == L_CRC)
            {
                STATE = CRC_DELIMITER;
            } 
        break;

        case CRC_DELIMITER:
            if(count == L_BIT)
            {
                STATE = ACK_SLOT;
            } 
        break;

        case ACK_SLOT:
            if(count == L_BIT)
            {
                STATE = ACK_DELIMITER;
            } 
        break;

        case ACK_DELIMITER:
            if(count == L_BIT)
            {
                STATE = EOF;
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
  UC_DECODER;

}
