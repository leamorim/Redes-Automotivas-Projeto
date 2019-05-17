#include <TimerOne.h>
#include <stdlib.h>

//QUASE LA
//STATES

#define SOF 0
#define ID_A 1
#define RTR 2
#define IDE 3
#define R0 4
#define DLC 5
#define DATA 6
#define crce 7
#define CRC_DELIMITER 8
#define ACK_SLOT 9
#define ACK_DELIMITER 10
#define EoF 11
#define INTERFRAME_SPACING 12
#define WAIT 13

//CONTROL SIGNALS

#define TQ 1000000
#define DATA_FRAME 1
#define REMOTE_FRAME 2
#define FRAME_TYPE REMOTE_FRAME

//FF = 0 -> Base Format / FF = 1 -> Extended Format
//FT = 1 -> Data Frame / FT = 2 -> Remote Frame / FT = 3 -> Error Frame / FT = 4 -> Overload Frame

volatile byte STATE;
int FF = 0; //FRAME FORMAT
int FT = 0; //FRAME TYPE
int Ecount = 0;
//char ID[11] = {'9','7','7','7','7','8','7','7','7','7','9'};
//char dlc[4] = {'D','L','C','E'};
//char crctest[15] = {'0','0','0','0','0','0','0','0','0','3','3','3','3','3','9'};
//char data[8] = {'0','1','1','1','1','1','1','0'};
char ID[11] = {'1','0','0','0','0','1','0','0','0','0','1'};
char dlc[4] = {'0','0','0','1'};
char data[8] = {'0','1','1','1','1','1','1','0'};
int DLC_L = 1;
//char *data =  malloc(sizeof(char) * (DLC_L*8));
//char *Frame = (char*) calloc(55,sizeof(char));
char *Frame = (char*) calloc(47 + DLC_L*8,sizeof(char));
char *printvec;

////Bit stuffing module variables

enum estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_ENC,STATE_DEC;
unsigned int count_encoder = 0;
unsigned int count_decoder = 0;
char last_bit_enc, last_bit_dec;
bool SEND_BIT = true;
bool BS_FLAG;
char BIT_TO_WRITE;


////// CRC CALCULATOR

char *MakeCRC(char *BitString)
   {
   static char Res[16];                                 // CRC Result
   char CRC[15];
   int  i;
   char DoInvert;
   Serial.println("BitString");
   Serial.println(BitString);
   
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
   Serial.println("Res");
   Serial.println(Res);
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
    char CAN_TX;
    //Falta colocar aqui if(Writing_Point) para só escrever quando tive num Writing_Point
    
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
        }
        else{ //count_encoder igual a 6
            //STATE_ENC = BIT_STUFFED;//dá pra criar um novo estado mas acho q dá pra deixar tudo nesse estado
           // Serial.println("BIT STUFFED");
            SEND_BIT = false;
            count_encoder = 1;
            last_bit_enc = BIT_TO_WRITE;
            if(BIT_TO_WRITE == '0'){
                CAN_TX = '1';
            }
            else if(BIT_TO_WRITE == '1'){
                CAN_TX = '0';
            }

            STATE_ENC = BIT_STUFFED;
        }
        break;

    case BIT_STUFFED:
        
        CAN_TX = last_bit_enc;
        SEND_BIT = true;
        if(BS_FLAG){
            STATE_ENC = COUNTING;
        }
        else{
            STATE_ENC = INACTIVE;
        }

        break;
    }
    Serial.print("CAN TX: ");
    Serial.println(CAN_TX);
    Serial.print("BIT TO WRITE: ");
    Serial.println(BIT_TO_WRITE);
    Serial.print("CAPTURE: ");
    Serial.println(SEND_BIT);
 /*   Serial.println("ordem: STATE_ENC/CAN_TX/count_encoder/SEND_BIT/last_bit/BIT_TO_WRITE");
    Serial.print(STATE_ENC);
    Serial.print("/");
    Serial.print(CAN_TX);
    Serial.print("/");
    Serial.print(count_encoder);
    Serial.print("/");
    Serial.print(SEND_BIT);
    Serial.print("/");
    Serial.print(last_bit_enc);
    Serial.print("/");
    Serial.print(BIT_TO_WRITE);
    Serial.println("/");
    */
    //return SEND_BIT;
}
////Bit Stuffing module





void setup() {
  Serial.begin(9600);
  STATE = SOF;
  STATE_DEC = INACTIVE;
  STATE_ENC = INACTIVE;
  //Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  //Timer1.attachInterrupt(Inc_Count);

}

void Frame_Printer(char*v, int ft,int ff,int dlc_l){
  Serial.println("FRAME:");
  int i;
  if(ft = DATA_FRAME){
  for(i = 0;i <(47 + (dlc_l*8));i++){
    Serial.print(v[i]);
  }
  }
  else if(ft = REMOTE_FRAME){
  for(int i = 0;i <47;i++){
    Serial.print(v[i]);
  }
  }
    Serial.println();
}

void Data_Builder(){
  char *crc;
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
      if(Ecount < 12){
        Frame[Ecount] = ID[Ecount-1];
      }
      else {
        STATE = RTR;
        Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
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
      if(Ecount < 19){
        Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = DATA;
        Ecount--;
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case DATA:
      if((Ecount < 19 + DLC_L*8) && (DLC_L != 0)){
        Frame[Ecount] = data[Ecount-19];
        Serial.print("COUNT DATA:  ");
        Serial.println(Ecount);
      }
      else {
        crc = MakeCRC(Frame);
        STATE = crce;
        Ecount--;
        Serial.print("COUNT DATA:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("DATA: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if((Ecount < 34 + DLC_L*8)){
        Frame[Ecount] = crc[Ecount - 19 -(DLC_L*8)];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = CRC_DELIMITER;
        BS_FLAG = false;
        Ecount--;
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
      //break;
    case CRC_DELIMITER:
      STATE = ACK_SLOT;
      Frame[Ecount] = 'D';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:
      STATE = ACK_DELIMITER;
      Frame[Ecount] = 'S';  // First Encoder writes RECESSIVE bit
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:
      STATE = EoF;
      Frame[Ecount] = 'A';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      if(Ecount < (44 + DLC_L*8)){
        Frame[Ecount] = '1';
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = INTERFRAME_SPACING;
        Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:
      if(Ecount < (47 + (DLC_L*8))){
        Frame[Ecount] = 'S';
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = WAIT;
        Ecount--;
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      break;     
      } 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }
      
}

void Remote_Builder(){
  char *crc;
  Serial.print("Remote Builder\n");
  if(SEND_BIT){
  switch(STATE){
    case SOF:
      Serial.print("COUNT SOF:  ");
      Serial.println(Ecount);
      STATE = ID_A;
      Frame[Ecount] = '0';
      BS_FLAG = true;
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:
      if(Ecount < 12){
        Frame[Ecount] = ID[Ecount-1];
      }
      else {
        STATE = RTR;
        Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
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
      if(Ecount < 19){
        Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
      }
      else {
        crc = MakeCRC(Frame);
        Serial.println("CRC AQUI");
        Serial.println(crc[0]);
        Serial.println(crc[3]);
        
        Serial.println(crc);
        
        STATE = crce;
        Ecount--;
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if(Ecount < 34){
        Frame[Ecount] = crc[Ecount - 19];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = CRC_DELIMITER;
        Ecount--;
        BS_FLAG = false;
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
      //break;
    case CRC_DELIMITER:
      STATE = ACK_SLOT;
      Frame[Ecount] = 'D';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:
      STATE = ACK_DELIMITER;
      Frame[Ecount] = 'S';  // First Encoder writes RECESSIVE bit
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:
      STATE = EoF;
      Frame[Ecount] = 'A';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      if(Ecount < 44){
        Frame[Ecount] = '1';
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = INTERFRAME_SPACING;
        Ecount--;
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
        Serial.print("COUNT EOF:  ");
        Serial.println(Ecount);
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:
      if(Ecount < 47){
        Frame[Ecount] = 'S';
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = WAIT;
        Ecount--;
        Serial.print("COUNT INTERFRAME SPACING:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      break;     
      }
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }
}

      
void Error_Builder();
void Overload_Builder();

void Frame_Builder(int FF,int FT){
  // Base Frame Builders
  if(FF == 0){
    switch(FRAME_TYPE){
      case DATA_FRAME:
        Data_Builder();
        break;

      case REMOTE_FRAME:
        Remote_Builder();
        break;

      /*case ERROR_FRAME:
        Error_Builder()
        break;

      case OVERLOAD_FRAME:
        Overload_Builder()
        break;
        */
    }
  }
  // Extended Frame Builders
  else{
    switch(FT){
      case DATA_FRAME:
        Data_Builder();
        break;
/*
      case REMOTE_FRAME:
        Remote_Builder()
        break;

      case ERROR_FRAME:
        Error_Builder()
        break;

      case OVERLOAD_FRAME:
        Overload_Builder()
        break;
        */
    }
  }
}

void loop() {
  Serial.flush();
  Frame_Builder(FF,FT);
  bit_stuffing_encoder();
  Serial.flush();
  Serial.print("FRAMEPRINT: ");
  Frame_Printer(Frame,FRAME_TYPE,0,DLC_L);
  Serial.flush();
  /*if(Ecount == 55) {
    Ecount = 0;
  }*/
  //delay(300);
}
  // put your main code here, to run repeatedly:
