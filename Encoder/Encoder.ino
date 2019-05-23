#include <TimerOne.h>
#include <stdlib.h>

//QUASE LA
// DATA AND REMOTE STATES

#define SOF   0
#define ID_A  1
#define RTR   2
#define IDE   3
#define R0    4
#define DLC   5
#define DATA  6
#define crce  7
#define CRC_DELIMITER       8
#define ACK_SLOT            9
#define ACK_DELIMITER       10
#define EoF                 11
#define INTERFRAME_SPACING  12

#define SRR 14
#define R1  15
#define R2  16
#define IDB 17

#define WAIT 13



//ERROR FRAME STATES
#define ERROR_FLAG_STATE 1
#define ERROR_DELIMITER 2

//OVERLOAD FRAME STATES
#define OVERLOAD_FLAG_STATE 1
#define OVERLOAD_DELIMITER 2


//CONTROL SIGNALS

#define TQ 1000000
#define DATA_FRAME 1
#define REMOTE_FRAME 2
#define ERROR_FRAME 3
#define OVERLOAD_FRAME 4
#define FRAME_TYPE REMOTE_FRAME

#define BASE 0
#define EXTENDED 1
#define FRAME_FORMAT EXTENDED

int DLC_L = 8;

//FF = 0 -> Base Format / FF = 1 -> Extended Format
//FT = 1 -> Data Frame / FT = 2 -> Remote Frame / FT = 3 -> Error Frame / FT = 4 -> Overload Frame

//VECTORS FOR TESTING WITH FICTIONAL VALUES FOR MORE ACCURATE DEBUGGING

volatile byte STATE;
int FF = FRAME_FORMAT; //FRAME FORMAT
int FT = FRAME_TYPE; //FRAME TYPE
int Ecount = 0;
//char ID[11] = {'9','7','7','7','7','8','7','7','7','7','9'};
//char ID[11] = {'I','I','I','I','I','I','I','I','I','I','I'};
//char dlc[4] = {'D','L','C','E'};
//char idb[18] = {'S','B','B','B','B','B','B','B','B','B','B','B','B','B','B','B','B','S'};

//char crctest[15] = {'C','3','3','3','3','3','3','R','3','3','3','3','3','3','C'};
//TEST CASE 1 - BASE DATA FRAME
//char ID[11] = {'1','1','0','0','1','1','1','0','0','1','0'};
//char dlc[4] = {'1','0','0','0'};
//char *data = "1010101010101010101010101010101010101010101010101010101010101010";
//char *idb = "110000000001111010";

//TEST CASE 2 - EXTENDED DATA FRAME
char *ID = "10001001001";
char *idb = "110000000001111010";
char dlc[4] = {'1','0','0','0'};
char *data = "0000001010101010101010101010101010101010101010101010101010101010";
//char *data = "10101010"; 01000100100111110000000001111010000100000000010101010101010101010101010101010101010101010101010101010100011100010011011111111111111
//01000100100100010000000001010101010101010101010101010101010101010101010101010101010 | 0111010000110001111111111111
// Stuffed - 01000100100110010000110010100111011111111111111
// Not Stuffed - 01000100100111110000010000111101010010000101001111101101111111111111
//0 | 10001001001 | 11 | 110000000001111010 | 100 | 1000 | 010100111110110 | 111 | 1111111111



//TEST CASE 5 - BASE DATA FRAME DLC 4
//char *ID = "11001110010";
//char *dlc = "0100";
//char *data = "10101010101010101010101010101010";

//TEST CASE 9 - BASE REMOTE FRAME DLC 0
/*char *ID = "11001110010";
char dlc[4] = {'0','0','0','0'};
char *data = "";
*/

//TEST CASE 13 - EXTENDED REMOTE FRAME DLC 0
/*/char *ID = "10001001001";
char dlc[4] = {'0','0','0','0'};
char *data = "";
char *idb = "110000000001111010";
*/


String printvec = "";
char CAN_TX;
char *crc;

char *Frame = NULL;


////Bit stuffing module variables

enum estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_ENC,STATE_DEC;
unsigned int count_encoder = 0;
unsigned int count_decoder = 0;
char last_bit_enc, last_bit_dec;
bool SEND_BIT = true;
bool BS_FLAG;
char BIT_TO_WRITE;
bool OK = true;


////// CRC CALCULATOR

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

//////// BIT STUFFING MODULE - ENCODER

void bit_stuffing_encoder(){

/* Entradas:
    Writing_Point
    ACK_Flag --> Entender o q eh q essa FLAG vai fazer
    BIT_TO_WRITE
    BS_FLAG
    Saídas:
    SEND_BIT --> Sinal que indica para o encoder enviar um novo bit ou não
*/
  
    //Falta colocar aqui if(Writing_Point) para só escrever quando tive num Writing_Point
 if(OK){   
    switch (STATE_ENC)
    {
    case INACTIVE:
        SEND_BIT = true;
        if(BS_FLAG){
            STATE_ENC = COUNTING;
            last_bit_enc = BIT_TO_WRITE;
            CAN_TX = BIT_TO_WRITE;
            count_encoder = 1;
        }
        else{
            STATE_ENC = INACTIVE;
            CAN_TX = BIT_TO_WRITE;
        }
        break;
    
    case COUNTING:
        if(!BS_FLAG){
          CAN_TX = BIT_TO_WRITE;
          STATE_ENC = INACTIVE;
          count_encoder = 0;
        }
        else{
            if(count_encoder < 5){
                if(BIT_TO_WRITE != last_bit_enc){
                    //Serial.println("diferente");
                    count_encoder = 1;
                    CAN_TX = BIT_TO_WRITE;
                    last_bit_enc = BIT_TO_WRITE;
                    SEND_BIT = true;
                }
                else{
                    //Serial.println("incremento");
                    count_encoder++;
                    //em tese as próximas duas linhas não são necessárias visto que BIT_TO_WRITE continua igual a last_bit
                    CAN_TX = BIT_TO_WRITE;
                    last_bit_enc = BIT_TO_WRITE;
                    SEND_BIT = true;
                }
                STATE_ENC = COUNTING;
            }
            else{ //count_encoder igual a 6
                //STATE_ENC = BIT_STUFFED;//dá pra criar um novo estado mas acho q dá pra deixar tudo nesse estado
               // Serial.println("BIT STUFFED");
                if(!BS_FLAG){
                  STATE = INACTIVE;
                }
                else{
                  if(BIT_TO_WRITE == last_bit_enc){
                    SEND_BIT = false;
                    count_encoder = 1;
                    
                    if(BIT_TO_WRITE == '0'){
                        CAN_TX = '1';
                    }
                    else if(BIT_TO_WRITE == '1'){
                        CAN_TX = '0';
                    }
                    STATE_ENC = BIT_STUFFED;
                  }
                  else {
                    SEND_BIT = true;
                    count_encoder = 1;
                    CAN_TX = BIT_TO_WRITE;
                    last_bit_enc = BIT_TO_WRITE;
                  }
                }
            }
        }
        break;

    case BIT_STUFFED:
        
        CAN_TX = last_bit_enc;
        count_encoder = 1;
        SEND_BIT = true;
        if(BS_FLAG){
            STATE_ENC = COUNTING;
        }
        else{
            STATE_ENC = INACTIVE;
        }
        break;
    }
 }
}

////Bit Stuffing module

void setup() {
  Serial.begin(9600);
  if(FT == DATA_FRAME || FT == REMOTE_FRAME){
    STATE = SOF; //In DATA/REMOTE FRAME CASES
  }
  else if(FT == ERROR_FRAME){
    STATE = ERROR_FLAG_STATE; //In ERROR FRAME CASES
  }
  else if (FT == OVERLOAD_FRAME){
    STATE = OVERLOAD_FLAG_STATE; //In ERROR FRAME CASES
  }
  STATE_DEC = INACTIVE;
  STATE_ENC = INACTIVE;
   if(FF == BASE){
      Frame = (char*) calloc(47 + DLC_L*8,sizeof(char));  //BASE FRAME CREATION
    }
    else if(FF == EXTENDED){
      Frame = (char*) calloc(67 + DLC_L*8,sizeof(char));  //EXTENDED FRAME CREATION
    }
    else if(FT == ERROR_FRAME || OVERLOAD_FRAME){
      Frame = (char*) calloc(14,sizeof(char)); 
    }
  //Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  //Timer1.attachInterrupt(Inc_Count);

}

void Frame_Printer(char*v,int ff,int ft,int dlc_l){
  if(ff == BASE){
   if(ft == DATA_FRAME){
      Serial.println("BASE DATA FRAME:");
      for(int i = 0;i <(47 + (dlc_l*8));i++){
        Serial.print(v[i]);
      }
    }
    else if(ft == REMOTE_FRAME){
      Serial.println("BASE REMOTE FRAME:");
      for(int i = 0;i <47;i++){
        Serial.print(v[i]);
      }
    }
    else if(ft == ERROR_FRAME || ft == OVERLOAD_FRAME ){
      Serial.println("BASE ERROR/OVERLOAD FRAME:");
      for(int i = 0;i <14;i++){
        Serial.print(v[i]);
      }
    }
  }
  else if(ff == EXTENDED){
    if(ft == DATA_FRAME){
      Serial.println("EXTENDED DATA FRAME:");
      for(int i = 0;i <(67 + (dlc_l*8));i++){
        Serial.print(v[i]);
      }
    }
    else if(ft == REMOTE_FRAME){
        Serial.println("EXTENDED REMOTE FRAME:");
      for(int i = 0;i <67;i++){
        Serial.print(v[i]);
      }
    }
  }
      Serial.println();
}

// BASE FRAME BUILDERS

void Data_Builder(int DLC_L){
  Serial.print("Data Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case SOF:
      Serial.print("COUNT SOF:  ");
      Serial.println(Ecount);
      STATE = ID_A;
      BS_FLAG = true;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:
      if(Ecount < 11){
        Frame[Ecount] = ID[Ecount-1];
      }
      else {
        //OK = false;
        Frame[Ecount] = ID[Ecount-1];   
        STATE = RTR;
        //Ecount--;
      }
      Serial.print("COUNT ID:  ");
      Serial.println(Ecount);
      Serial.print("IDA: ");
      Serial.println(Frame[Ecount]);
      break;
    case RTR:
      Frame[Ecount] = '0'; // 12 position
      STATE = IDE;
      Serial.print("COUNT RTR:  ");
      Serial.println(Ecount);
      Serial.print("RTR: ");
      Serial.println(Frame[Ecount]);
      break;
    case IDE:
      Frame[Ecount] = '0';  // 13 position
      STATE = R0;
      Serial.print("COUNT IDE:  ");
      Serial.println(Ecount);
      Serial.print("IDE: ");
      Serial.println(Frame[Ecount]);
      break;
    case R0:
      Frame[Ecount] = '0';   // 14 position
      STATE = DLC;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 18){
        Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
      }
      else {
        //OK = false;
        Frame[Ecount] = dlc[Ecount-15];
        STATE = DATA;
        //Ecount--;
        /*Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case DATA:
      BS_FLAG = true;
      if((Ecount < 18 + DLC_L*8) && (DLC_L != 0)){
        Frame[Ecount] = data[Ecount-19];
        Serial.print("COUNT DATA:  ");
        Serial.println(Ecount);
      }
      else {
        //OK = false;
        Frame[Ecount] = data[Ecount-19];
        crc = MakeCRC(Frame);
        Serial.println("CRC" );
        Serial.print(crc);
        STATE = crce;
        //Ecount--;
        /*Serial.print("COUNT DATA:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("DATA: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if((Ecount < 33 + DLC_L*8)){
        Frame[Ecount] = crc[Ecount - 19 -(DLC_L*8)];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        //OK = false;
        Frame[Ecount] = crc[Ecount - 19 -(DLC_L*8)];
        STATE = CRC_DELIMITER;
        BS_FLAG = false;
        //Ecount--;
        /*Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
        */
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:
      BS_FLAG = false;
      STATE = ACK_SLOT;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:
      BS_FLAG = false;
      STATE = ACK_DELIMITER;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:
      BS_FLAG = false;
      STATE = EoF;
      Frame[Ecount] = '1';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:

      BS_FLAG = false;
      if(Ecount < (43 + DLC_L*8)){
        Frame[Ecount] = '1';
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      else {
        //OK = false;
        Frame[Ecount] = '1';
        STATE = INTERFRAME_SPACING;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        /*Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
        */
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:
      BS_FLAG = false;
      if(Ecount < (46 + (DLC_L*8))){
        Frame[Ecount] = '1';
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
      }
      else {
        //OK = false;
        Frame[Ecount] = '1';
        STATE = WAIT;
        //Ecount--;
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE = WAIT;
      while(1);
      break;     
      } 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }
      
}

void Remote_Builder(){
  Serial.print("Remote Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case SOF:
      Serial.print("COUNT SOF:  ");
      Serial.println(Ecount);
      STATE = ID_A;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:
      if(Ecount < 11){
        Frame[Ecount] = ID[Ecount-1];
      }
      else {
        Frame[Ecount] = ID[Ecount-1];
        STATE = RTR;
        //Ecount--;
        //OK = false;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("COUNT ID:  ");
      Serial.println(Ecount);
      Serial.print("IDA: ");
      Serial.println(Frame[Ecount]);
      break;
    case RTR:
      Frame[Ecount] = '1'; // 12 position
      STATE = IDE;
      Serial.print("COUNT RTR:  ");
      Serial.println(Ecount);
      Serial.print("RTR: ");
      Serial.println(Frame[Ecount]);
      break;
    case IDE:
      Frame[Ecount] = '0';  // 13 position
      STATE = R0;
      Serial.print("COUNT IDE:  ");
      Serial.println(Ecount);
      Serial.print("IDE: ");
      Serial.println(Frame[Ecount]);
      break;
    case R0:
      Frame[Ecount] = '0';   // 14 position
      STATE = DLC;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 18){
        Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = dlc[Ecount-15];
        //OK = false;
        crc = MakeCRC(Frame);
        /*Serial.println("CRC AQUI");
        Serial.println(crc[0]);
        Serial.println(crc[3]);
        
        Serial.println(crc);
        */
        
        STATE = crce;
        //Ecount--;
        /*Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if(Ecount < 33){
        Frame[Ecount] = crc[Ecount - 19];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = crc[Ecount - 19];
        //OK = false;
        STATE = CRC_DELIMITER;
        //Ecount--;
        /*Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
        */
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:
      BS_FLAG = false;
      STATE = ACK_SLOT;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:
      BS_FLAG = false;
      STATE = ACK_DELIMITER;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:
      BS_FLAG = false;
      STATE = EoF;
      Frame[Ecount] = '1';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      BS_FLAG = false;
      if(Ecount < 43){
        Frame[Ecount] = '1';
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = '1';
        //OK = false;
        STATE = INTERFRAME_SPACING;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        /*Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
        */
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:
      BS_FLAG = false;
      if(Ecount < 46){
        Frame[Ecount] = '1';
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = '1';
        //OK = false;
        STATE = WAIT;
        //Ecount--;
        /*Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE = WAIT;
      while(1);
      break;     
      }
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }
}

//EXTENDED FRAME CONSTRUCTORS

void Ex_Data_Builder(int DLC_L){
  Serial.print("Extended Data Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case SOF:
      Serial.print("COUNT SOF:  ");
      Serial.println(Ecount);
      STATE = ID_A;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:        // 1 - 11 position
      if(Ecount < 11){
        Frame[Ecount] = ID[Ecount-1];
      }
      else {
        Frame[Ecount] = ID[Ecount-1];
        //OK = false;
        STATE = SRR;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("COUNT ID:  ");
      Serial.println(Ecount);
      Serial.print("IDA: ");
      Serial.println(Frame[Ecount]);
      break;
    case SRR:
      Frame[Ecount] = '1'; // 12 position
      STATE = IDE;
      Serial.print("COUNT RTR:  ");
      Serial.println(Ecount);
      Serial.print("RTR: ");
      Serial.println(Frame[Ecount]);
      break;
    case IDE:
      Frame[Ecount] = '1';  // 13 position
      STATE = IDB;
      Serial.print("COUNT IDE:  ");
      Serial.println(Ecount);
      Serial.print("IDE: ");
      Serial.println(Frame[Ecount]);
      break;
    case IDB:
    if(Ecount < 31){
        Frame[Ecount] = idb[Ecount-14]; //14 - 31 position
      }
      else {
        Frame[Ecount] = idb[Ecount-14];
        //OK = false;
        STATE = RTR;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("COUNT ID:  ");
      Serial.println(Ecount);
      Serial.print("IDA: ");
      Serial.println(Frame[Ecount]);
      break;
      
    case RTR:
      Frame[Ecount] = '0';   // 32 position
      STATE = R1;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R1:
      Frame[Ecount] = '0';   // 33 position
      STATE = R2;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R2:
      Frame[Ecount] = '0';   // 34 position
      STATE = DLC;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 38){
        Frame[Ecount] = dlc[Ecount-35];   // 35 - 38
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = dlc[Ecount-35];
        //OK = false;
        STATE = DATA;
        //Ecount--;
        /*Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case DATA:
      if((Ecount < 38 + (DLC_L*8)) && (DLC_L != 0)){
          Frame[Ecount] = data[Ecount-39];   // 35 (+DLC_L) - 38 (+DLC_L)
          Serial.print("COUNT DATA:  ");
          Serial.println(Ecount);
        }
        else {
          Frame[Ecount] = data[Ecount-39];
          //OK = false;
          crc = MakeCRC(Frame); // N esta funcionando no momento
          STATE = crce;
          //Ecount--;
          /*Serial.print("COUNT ATA:  ");
          Serial.println(Ecount);
          */
          //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        }
        Serial.print("DATA: ");
        Serial.println(Frame[Ecount]);
        break;
    case crce:
      if(Ecount < 53 + (DLC_L*8)){      // 39(+DLC_L) - 53(+DLC_L) Position
        Frame[Ecount] = crc[Ecount - 39 - (DLC_L*8)];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = crc[Ecount - 39 - (DLC_L*8)];
        //OK = false;
        STATE = CRC_DELIMITER;
        //Ecount--;
        BS_FLAG = false;
        /*Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
      //break;
    case CRC_DELIMITER:       //54 (+DLC_L) Position
      STATE = ACK_SLOT;
      BS_FLAG = false;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:            //55 (+DLC_L) Position
      STATE = ACK_DELIMITER;
      BS_FLAG = false;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:        //56 (+DLC_L) Position
      STATE = EoF;
      Frame[Ecount] = '1';
      BS_FLAG = false;      
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      BS_FLAG = false;
      if(Ecount < 63 + (DLC_L*8)){        // 57 (+DLC_L) - 63 (+DLC_L)
        Frame[Ecount] = '1';
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = '1';
        //OK = false;
        STATE = INTERFRAME_SPACING;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        /*Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
        */
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:    //64 (+DLC_L) - 66 (+DLC_L) Position
      BS_FLAG = false;
      if(Ecount < 66 + (DLC_L*8)){
        Frame[Ecount] = '1';
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = '1';
        //OK = false;
        STATE = WAIT;
        //Ecount--;
        /*Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE = WAIT;
      while(1);
      break;     
      }
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }
}

void Ex_Remote_Builder(){
  Serial.print("Extended Remote Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case SOF:
      Serial.print("COUNT SOF:  ");
      Serial.println(Ecount);
      STATE = ID_A;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:        // 1 - 11 position
      if(Ecount < 11){
        Frame[Ecount] = ID[Ecount-1];
      }
      else {
        Frame[Ecount] = ID[Ecount-1];
        //OK = false;
        STATE = SRR;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("COUNT ID:  ");
      Serial.println(Ecount);
      Serial.print("IDA: ");
      Serial.println(Frame[Ecount]);
      break;
    case SRR:
      Frame[Ecount] = '1'; // 12 position
      STATE = IDE;
      Serial.print("COUNT RTR:  ");
      Serial.println(Ecount);
      Serial.print("RTR: ");
      Serial.println(Frame[Ecount]);
      break;
    case IDE:
      Frame[Ecount] = '1';  // 13 position
      STATE = IDB;
      Serial.print("COUNT IDE:  ");
      Serial.println(Ecount);
      Serial.print("IDE: ");
      Serial.println(Frame[Ecount]);
      break;
    case IDB:
    if(Ecount < 31){
        Frame[Ecount] = idb[Ecount-14]; //14 - 31 position
      }
      else {
        Frame[Ecount] = idb[Ecount-14];
        //OK = false;
        STATE = RTR;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("COUNT ID:  ");
      Serial.println(Ecount);
      Serial.print("IDA: ");
      Serial.println(Frame[Ecount]);
      break;
      
    case RTR:
      Frame[Ecount] = '1';   // 32 position
      STATE = R1;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R1:
      Frame[Ecount] = '0';   // 33 position
      STATE = R2;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R2:
      Frame[Ecount] = '0';   // 34 position
      STATE = DLC;
      Serial.print("COUNT RZERO:  ");
      Serial.println(Ecount);
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 38){
        Frame[Ecount] = dlc[Ecount-35];   // 35 - 38
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = dlc[Ecount-35];
        //OK = false;
        crc = MakeCRC(Frame);
        STATE = crce;
        //Ecount--;
        /*Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if(Ecount < 53){      // 39 - 53 Position
        Frame[Ecount] = crc[Ecount - 39];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = crc[Ecount - 39];
        //OK = false;
        STATE = CRC_DELIMITER;
        //Ecount--;
        /*Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:       //54 Position
      BS_FLAG = false;
      STATE = ACK_SLOT;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:            //55 Position
      BS_FLAG = false;
      STATE = ACK_DELIMITER;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:        //56 Position
      BS_FLAG = false;
      STATE = EoF;
      Frame[Ecount] = '1';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      BS_FLAG = false;
      if(Ecount < 63){        // 57-63
        Frame[Ecount] = '1';
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = '1';
        //OK = false;
        STATE = INTERFRAME_SPACING;
        //Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        /*Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
        */
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:    //64 - 66 Position
      BS_FLAG = false;
      if(Ecount < 66){
        Frame[Ecount] = '1';
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
      }
      else {
        Frame[Ecount] = '1';
        //OK = false;
        STATE = WAIT;
        //Ecount--;
        /*Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
        */
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE = WAIT;
      while(1);
      break;     
      }
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }
}

//ERROR FRAME CONSTRUCTOR

void Error_Builder(){
  Serial.print("Error Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case ERROR_FLAG_STATE:
      if(Ecount < 5){
          Frame[Ecount] = '0';
          Serial.print("COUNT ERROR FLAG:  ");
          Serial.println(Ecount);
        }
        else {
          Frame[Ecount] = '0';
          //OK = false;
          STATE = ERROR_DELIMITER;
          //Ecount--;
          /*Serial.print("COUNT ERROR FLAG:  ");
          Serial.println(Ecount);
          */
          //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        }
        BS_FLAG = false;
        Serial.print("ERROR_FLAG_STATE: ");
        Serial.println(Frame[Ecount]);
        break;
    case ERROR_DELIMITER:
    BS_FLAG = false;
      if(Ecount < 14){
            Frame[Ecount] = '1';
            Serial.print("COUNT ERROR DELIMITER:  ");
            Serial.println(Ecount);
          }
          else {
            Frame[Ecount] = '1';
            //OK = false;
            STATE = WAIT;
            //Ecount--;
            /*Serial.print("COUNT ERROR DELIMITER:  ");
            Serial.println(Ecount);
            */
            //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
          }
          Serial.print("ERROR_DELIMITER: ");
          Serial.println(Frame[Ecount]);
          break;
    case WAIT:
      Serial.println("FRAME END");
      STATE = WAIT;
      while(1);
      break;     
      } 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }  
}

//OVERLOAD FRAME CONSTRUCTOR

void Overload_Builder(){
  Serial.print("Overload Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case OVERLOAD_FLAG_STATE:
      if(Ecount < 5){
          Frame[Ecount] = '0';
          Serial.print("COUNT OVERLOAD FLAG:  ");
          Serial.println(Ecount);
        }
        else {
          Frame[Ecount] = '0';
          STATE = OVERLOAD_DELIMITER;
          //Ecount--;
          Serial.print("COUNT OVERLOAD FLAG:  ");
          Serial.println(Ecount);
          //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        }
        BS_FLAG = false;
        Serial.print("OVERLOAD FLAG: ");
        Serial.println(Frame[Ecount]);
        break;
    case OVERLOAD_DELIMITER:
      if(Ecount < 14){
            Frame[Ecount] = '1';
            Serial.print("COUNT OVERLOAD DELIMITER:  ");
            Serial.println(Ecount);
          }
          else {
            Frame[Ecount] = '1';
            STATE = WAIT;
            //Ecount--;
            Serial.print("COUNT OVERLOAD DELIMITER:  ");
            Serial.println(Ecount);
            //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
          }
          Serial.print("OVERLOAD_DELIMITER: ");
          Serial.println(Frame[Ecount]);
          break;
    case WAIT:
      Serial.println("FRAME END");
      while(1);
      break;     
      } 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }  
}      

void Frame_Builder(int FF,int FT,int DLC_L){
  OK = true;
  BS_FLAG = true;
  // Base Frame Builders
  if(DLC_L > 8){
    DLC_L = 8;
    }
  if(FF == 0){
    switch(FT){
      case DATA_FRAME:
        Data_Builder(DLC_L);
        break;

      case REMOTE_FRAME:
        Remote_Builder();
        break;

      case ERROR_FRAME:
        Error_Builder();
        break;
      case OVERLOAD_FRAME:
        Overload_Builder();
        break;
    }
  }
  // Extended Frame Builders
  else{
    switch(FT){
      case DATA_FRAME:
        Ex_Data_Builder(DLC_L);
        break;
        
      case REMOTE_FRAME:
        Ex_Remote_Builder();
        break;
        
      case ERROR_FRAME:
        Error_Builder();
        break;
        
      case OVERLOAD_FRAME:
        Overload_Builder();
        break;
    }
  }
}

void loop() {
  Serial.flush();
  Frame_Builder(FF,FT,DLC_L);
  bit_stuffing_encoder();
  Serial.println("Bit Stuffed Frame: ");
  //if(OK){
   printvec.concat(CAN_TX);
 // }
  Serial.println(printvec);
  Serial.flush();
  Serial.print("FRAMEPRINT: ");
  Frame_Printer(Frame,FF,FT,DLC_L);
  Serial.flush();
  //delay(300);
}
