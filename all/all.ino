//Bibliotecas e Defines BEGIN
#include <TimerOne.h>
#include <SoftwareSerial.h>

#define CAN_RX_PIN 11
#define CAN_TX_PIN 10
SoftwareSerial mySerial(CAN_RX_PIN,CAN_TX_PIN);

    //Bit_Timing Defines
#define TQ 100000  //Tempo em Microssegundos
#define L_SYNC 1
#define L_PROP 1
#define L_PH_SEG1 2
#define L_PH_SEG2 3
#define SJW 1
/// Tamanhos de L_SEG1 E L_SEG2, saídas do módulo TQ_Configurator
#define L_SEG1 (L_PROP+L_PH_SEG1)
#define L_SEG2 L_PH_SEG2

enum dec_estados {BUS_IDLE = 0,SoF = 1,ID_A = 2,RTR_SRR = 3,IDE_0 = 4,R0 = 5, DLC = 6,
DATA = 7, CRC_READ = 8,CRC_DELIMITER = 9, ACK_SLOT = 10, ACK_DELIMITER, EoF,
INTERFRAME_SPACING,IDE_1, ID_B,RTR, R1R0, STATE_ERROR,
FORMAT_ERROR, ACK_ERROR, CRC_ERROR, BIT_STUFFING_ERROR, STATE_BSD_FLAG1,
BIT_ERROR , STATE_BED_FLAG1, OVERLOAD, WAIT , SOF, IDE, crce, SRR,R1,R2,
IDB,ERROR_FLAG_STATE, ERROR_DELIMITER,
OVERLOAD_DELIMITER, OVERLOAD_FLAG_STATE, ARBITRATION_LOSS_STATE} STATE_DEC, STATE_ENC;

enum send_frame_states {FORMAT_SEND = 0, TYPE_SEND, ID_A_SEND, ID_B_SEND, DATA_SEND, WAIT_SEND, DLC_SEND} STATE_SEND;

    //Decoder Defines
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

  // Decoder teste
  char *ID_A_DECODER = "10001001001";         //0x0449
  char *ID_B_DECODER = "110000000001111010"; //0x3007A


    //Encoder Defines
//Sinais de controle para construção de um frame
#define DATA_FRAME 1
#define REMOTE_FRAME 2
#define ERROR_FRAME 3
#define OVERLOAD_FRAME 4
#define FRAME_TYPE REMOTE_FRAME

#define BASE 0
#define EXTENDED 1
#define FRAME_FORMAT EXTENDED

//Bibliotecas e Defines END


/****** TESTES ******/
//Encoder teste -- DADOS BASE
/*  char *ID = "10001001001";         //0x449
  char *idb = "110000000001111010"; //0x3007A
  char dlc[4] = {'1','0','0','0'};  //8 bytes
  char *data = "1010101010101010101010101010101010101010101010101010101010101010";// 0xAAAAAAAAAAAAAAAA
*/
/*  //Encoder teste -- DADOS BASE
  char *ID = "11001110010";         //0x672
  char *idb = "110000000001111010"; //0x3007A
  char dlc[4] = {'1','0','0','0'};  //8 bytes
  char *data = "1010101010101010101010101010101010101010101010101010101010101010";// 0xAAAAAAAAAAAAAAAA

  //Encoder teste -- REMOTO BASE
  char *ID = "11001110010";         //0x672
  char *idb = "110000000001111010"; //0x3007A
  char dlc[4] = {'1','0','0','0'};  //8 bytes
  char *data = "1010101010101010101010101010101010101010101010101010101010101010";// 0xAAAAAAAAAAAAAAAA
*/


//Extended Remote FRAME
  char ID [11] = "";         //0x67E
  char idb [18] = ""; //0x3187A
  char dlc[5] = "";  //8 
  char data [64] = "";// 0xAAAAAAAAAAAAAAAA

  int DLC_L;
  int FF ; //FRAME FORMAT
  int FT ; //FRAME TYPE
  bool FRAME_START = true;
/****** TESTES ******/



//Variáveis Globais BEGIN
    //Bit_Timing Variáveis
enum bt_estados {SYNC = 0,SEG1 = 1,SEG2 = 2} STATE_BT;
unsigned int count_bt = 0;
int Ph_Error = 0;
volatile bool Plot_Tq = false;
volatile bool Sample_Point = false;
volatile bool Writing_Point = false;
volatile bool Soft_Sync = false;
volatile bool Hard_Sync = false;
volatile bool SS_Flag = false;
volatile char last_bit_bt = '\0';


//Decoder Variáveis BEGIN
 
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

  unsigned int Data_Flag = 0;
  unsigned int Remote_Flag = 0;
  unsigned int Extended_Flag = 0; // 0-> Base || 1 -> Extended

  char Vetor_ID_A[11];
  char Vetor_DLC[4];
  char Vetor_ID_B[18];
  char Vetor_DATA[64];
  char Vetor_CRC[15];
  char *Resultado_CRC;
  char Vetor_Frame[133];
  int count_frame = 0;

  long int Value_ID_A;
  long int Value_DLC;
  long int Value_ID_B;
  long int Value_CRC;
  char DataHex[4];
  int i;

  unsigned int error_count = 0;
  bool error_12 = false;
  int count_6_12 = 0;
  bool OVERLOAD_FLAG_1 = false;

  
//Decoder Variáveis END

  // Bit Stuffing Decoder BEGIN
    enum bs_estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_BS_ENC,STATE_BS_DEC;
    unsigned int count_bs_encoder = 0;
    unsigned int count_bs_decoder = 0;
    char last_bit_dec;

    char BIT_TO_SAVE = '\0';
    bool CAPTURE,BSE_FLAG, BSD_FLAG = true; 
  // Bit Stuffing Decoder END

//Encoder Variáveis

    bool ARBITRATION_LOSS =  false;//Indica a perca de Arbitração, este valor é entrada do Frame builder e saída do BS
    bool ACK_SLOT_FLAG = false;//Sinal enviado do Frame_Builder para o BS para indicar que o campo atual é o de ACK_SLOT
    bool ACK_CONFIRM =  true;//Sinal enviado do BS para o Frame_Builder para indicar que o ACK foi recebido com sucesso

    int Ecount = 0;
    char CAN_TX = '\0';
    char CAN_RX = '\0';
    char *crc;
    char *Frame = NULL;
    String printvec = ""; //printa o frame montado após o módulo de Bit Stuffing

    //Variáveis BS Encoder

    unsigned int count_encoder = 0;
    char last_bit_enc;
    bool SEND_BIT = true;
    bool BS_FLAG;
    char BIT_TO_WRITE;


//Variáveis Globais END


//CRC_Module BEGIN

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
//CRC_Module END


//Encoder BEGIN

  //Bit Stuffing Encoder BEGIN
  
void bit_stuffing_encoder(){

 //Entradas:
 //   Writing_Point
 //   ACK_Flag --> Entender o q eh q essa FLAG vai fazer
 //   BIT_TO_WRITE
  //  BS_FLAG
  //  Saídas:
  //  SEND_BIT --> Sinal que indica para o encoder enviar um novo bit ou não

  
    //Falta colocar aqui if(Writing_Point) para só escrever quando tive num Writing_Point
    switch (STATE_BS_ENC)
    {
    case INACTIVE:
        SEND_BIT = true;
        if(BS_FLAG){
            STATE_BS_ENC = COUNTING;
            last_bit_enc = BIT_TO_WRITE;
            CAN_TX = BIT_TO_WRITE;
            count_encoder = 1;
        }
        else{
            STATE_BS_ENC = INACTIVE;
            CAN_TX = BIT_TO_WRITE;
        }
        break;
    
    case COUNTING:
        if(!BS_FLAG){
          CAN_TX = BIT_TO_WRITE;
          STATE_BS_ENC = INACTIVE;
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
                STATE_BS_ENC = COUNTING;
            }
            else{ //sexto bit aqui
                //STATE_BS_ENC_ENC = BIT_STUFFED;//dá pra criar um novo estado mas acho q dá pra deixar tudo nesse estado
               // Serial.println("BIT STUFFED");
                if(!BS_FLAG){
                  STATE_BS_ENC = INACTIVE;
                }
                else{
                    SEND_BIT = false;
                    count_encoder = 1;
                    if(BIT_TO_WRITE == '0'){
                        CAN_TX = '1';
                    }
                    else if(BIT_TO_WRITE == '1'){
                        CAN_TX = '0';
                    }
                    STATE_BS_ENC = BIT_STUFFED;
                    last_bit_enc = BIT_TO_WRITE;
                  }
                }
          }
        break;

    case BIT_STUFFED:
        
        CAN_TX = last_bit_enc;
        count_encoder = 1;
        SEND_BIT = true;
        if(BS_FLAG){
            STATE_BS_ENC = COUNTING;
        }
        else{
            STATE_BS_ENC = INACTIVE;
        }
        break;
    }
 }
  //Bit Stuffing Encoder FIM



void Frame_Printer(char*v,int ff,int ft,int dlc_l){
  if(ff == BASE){
   if(ft == DATA_FRAME){
      for(int i = 0;i <(47 + (dlc_l*8));i++){
        Serial.print(v[i]);
      }
    }
    else if(ft == REMOTE_FRAME){
      for(int i = 0;i <47;i++){
        Serial.print(v[i]);
      }
    }
    else if(ft == ERROR_FRAME || ft == OVERLOAD_FRAME ){
      for(int i = 0;i <14;i++){
        Serial.print(v[i]);
      }
    }
  }
  else if(ff == EXTENDED){
    if(ft == DATA_FRAME)
      for(int i = 0;i <(67 + (dlc_l*8));i++){
        Serial.print(v[i]);
      }
    }
    else if(ft == REMOTE_FRAME){
      for(int i = 0;i <67;i++){
        Serial.print(v[i]);
      }
    }
}

// BASE FRAME BUILDERS

void Data_Builder(int DLC_L){
  if(SEND_BIT){
  switch(STATE_ENC){
    case ARBITRATION_LOSS_STATE:
      if (BUS_IDLE){
        STATE_ENC = SOF;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
      break; 
    case SOF:
      Ecount = 0;
      STATE_ENC = ID_A;
      BS_FLAG = true;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:
      if(!ARBITRATION_LOSS){
        if(Ecount < 11){
          Frame[Ecount] = ID[Ecount-1];
        }
        else {
          Frame[Ecount] = ID[Ecount-1];   
          STATE_ENC = RTR;
        }
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("IDA: ");
        Serial.println(Frame[Ecount]);
        break;
    case RTR:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '0'; // 12 position
        STATE_ENC = IDE;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("RTR: ");
        Serial.println(Frame[Ecount]);
        break;
    case IDE:
      Frame[Ecount] = '0';  // 13 position
      STATE_ENC = R0;
      Serial.print("IDE: ");
      Serial.println(Frame[Ecount]);
      break;
    case R0:
      Frame[Ecount] = '0';   // 14 position
      STATE_ENC = DLC;
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 18){
        Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
      }
      else {
        Frame[Ecount] = dlc[Ecount-15];
        STATE_ENC = DATA;
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case DATA:
      BS_FLAG = true;
      if((Ecount < 18 + DLC_L*8) && (DLC_L != 0)){
        Frame[Ecount] = data[Ecount-19];
      }
      else {
        Frame[Ecount] = data[Ecount-19];
        crc = MakeCRC(Frame);
        Serial.println("CRC" );
        Serial.print(crc);
        STATE_ENC = crce;
      }
      Serial.print("DATA: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if((Ecount < 33 + DLC_L*8)){
        Frame[Ecount] = crc[Ecount - 19 -(DLC_L*8)];
      }
      else {
        Frame[Ecount] = crc[Ecount - 19 -(DLC_L*8)];
        STATE_ENC = CRC_DELIMITER;
        BS_FLAG = false;
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:
      BS_FLAG = false;
      STATE_ENC = ACK_SLOT;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:
      ACK_SLOT_FLAG = true;
      BS_FLAG = false;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      if(ACK_CONFIRM){
        STATE_ENC = ACK_DELIMITER;
      }
      else{
        STATE_ENC = SOF;
      }
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:
      BS_FLAG = false;
      STATE_ENC = EoF;
      Frame[Ecount] = '1';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:

      BS_FLAG = false;
      if(Ecount < (43 + DLC_L*8)){
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = INTERFRAME_SPACING;
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:
      BS_FLAG = false;
      if(Ecount < (46 + (DLC_L*8))){
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = WAIT;
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE_ENC = WAIT;
      FRAME_START = true;
  //      while(1);//Bloqueia no fim do frame
      break;
    }
      if(!ARBITRATION_LOSS){     
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
      }
  }
      
}

void Remote_Builder(){
  if(SEND_BIT){
  switch(STATE_ENC){
    case ARBITRATION_LOSS_STATE:
      if(BUS_IDLE){
        STATE_ENC = SOF;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
      break; 
    case SOF:
      Ecount = 0;
      STATE_ENC = ID_A;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:
      if(!ARBITRATION_LOSS){
        if(Ecount < 11){
          Frame[Ecount] = ID[Ecount-1];
        }
        else {
          Frame[Ecount] = ID[Ecount-1];
          STATE_ENC = RTR;
        }
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("IDA: ");
        Serial.println(Frame[Ecount]);
        break;
    case RTR:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '1'; // 12 position
        STATE_ENC = IDE;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("RTR: ");
        Serial.println(Frame[Ecount]);
        break;
    case IDE:
      Frame[Ecount] = '0';  // 13 position
      STATE_ENC = R0;
      Serial.print("IDE: ");
      Serial.println(Frame[Ecount]);
      break;
    case R0:
      Frame[Ecount] = '0';   // 14 position
      STATE_ENC = DLC;
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 18){
        Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
      }
      else {
        Frame[Ecount] = dlc[Ecount-15];
        crc = MakeCRC(Frame);
        STATE_ENC = crce;
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if(Ecount < 33){
        Frame[Ecount] = crc[Ecount - 19];
      }
      else {
        Frame[Ecount] = crc[Ecount - 19];
        STATE_ENC = CRC_DELIMITER;
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:
      BS_FLAG = false;
      STATE_ENC = ACK_SLOT;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:
      ACK_SLOT_FLAG = true;
      BS_FLAG = false;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      if(ACK_CONFIRM){
        STATE_ENC = ACK_DELIMITER;
      }
      else{
        STATE_ENC = SOF;
      }
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:
      BS_FLAG = false;
      STATE_ENC = EoF;
      Frame[Ecount] = '1';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      BS_FLAG = false;
      if(Ecount < 43){
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = INTERFRAME_SPACING;
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:
      BS_FLAG = false;
      if(Ecount < 46){
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = WAIT;
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE_ENC = WAIT;
      FRAME_START = true;
      //while(1);
      break;     
    }
      if(!ARBITRATION_LOSS){ 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
      }
  }
}

//EXTENDED FRAME CONSTRUCTORS

void Ex_Data_Builder(int DLC_L){
  if(SEND_BIT){
  switch(STATE_ENC){
    case ARBITRATION_LOSS_STATE:
      if (BUS_IDLE){
        STATE_ENC = SOF;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
      break; 
    case SOF:
      Ecount = 0;
      STATE_ENC = ID_A;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:        // 1 - 11 position
      if(!ARBITRATION_LOSS){
          if(Ecount < 11){
            Frame[Ecount] = ID[Ecount-1];
          }
          else {
            Frame[Ecount] = ID[Ecount-1];
            STATE_ENC = SRR;
          }
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
        Serial.print("IDA: ");
        Serial.println(Frame[Ecount]);
        break;
    case SRR:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '1'; // 12 position
        STATE_ENC = IDE;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("RTR: ");
        Serial.println(Frame[Ecount]);
        break;
    case IDE:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '1';  // 13 position
        STATE_ENC = IDB;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("IDE: ");
        Serial.println(Frame[Ecount]);
        break;
    case IDB:
      if(!ARBITRATION_LOSS){
        if(Ecount < 31){
            Frame[Ecount] = idb[Ecount-14]; //14 - 31 position
          }
          else {
            Frame[Ecount] = idb[Ecount-14];
            STATE_ENC = RTR;
          }
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
          Serial.print("IDB: ");
          Serial.println(Frame[Ecount]);
          break;
          
    case RTR:
    if(!ARBITRATION_LOSS){
      Frame[Ecount] = '0';   // 32 position
      STATE_ENC = R1;
    }
    else{
      STATE_ENC = ARBITRATION_LOSS_STATE;
    }
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R1:
      Frame[Ecount] = '0';   // 33 position
      STATE_ENC = R2;
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R2:
      Frame[Ecount] = '0';   // 34 position
      STATE_ENC = DLC;
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 38){
        Frame[Ecount] = dlc[Ecount-35];   // 35 - 38
      }
      else {
        Frame[Ecount] = dlc[Ecount-35];
        STATE_ENC = DATA;
      }
      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case DATA:
      if((Ecount < 38 + (DLC_L*8)) && (DLC_L != 0)){
          Frame[Ecount] = data[Ecount-39];   // 35 (+DLC_L) - 38 (+DLC_L)
        }
        else {
          Frame[Ecount] = data[Ecount-39];
          crc = MakeCRC(Frame); // N esta funcionando no momento
          STATE_ENC = crce;
        }
        Serial.print("DATA: ");
        Serial.println(Frame[Ecount]);
        break;
    case crce:
      if(Ecount < 53 + (DLC_L*8)){      // 39(+DLC_L) - 53(+DLC_L) Position
        Frame[Ecount] = crc[Ecount - 39 - (DLC_L*8)];
      }
      else {
        Frame[Ecount] = crc[Ecount - 39 - (DLC_L*8)];
        STATE_ENC = CRC_DELIMITER;
        BS_FLAG = false;
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:       //54 (+DLC_L) Position
      STATE_ENC = ACK_SLOT;
      BS_FLAG = false;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:            //55 (+DLC_L) Position
      ACK_SLOT_FLAG = true;
      BS_FLAG = false;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      if(ACK_CONFIRM){
        STATE_ENC = ACK_DELIMITER;
      }
      else{
        STATE_ENC = SOF;
      }
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:        //56 (+DLC_L) Position
      STATE_ENC = EoF;
      Frame[Ecount] = '1';
      BS_FLAG = false;      
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      BS_FLAG = false;
      if(Ecount < 63 + (DLC_L*8)){        // 57 (+DLC_L) - 63 (+DLC_L)
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = INTERFRAME_SPACING;
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:    //64 (+DLC_L) - 66 (+DLC_L) Position
      BS_FLAG = false;
      if(Ecount < 66 + (DLC_L*8)){
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = WAIT;
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE_ENC = WAIT;
      FRAME_START = true;
      //while(1);
      break;     
      }
      if(!ARBITRATION_LOSS){     
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
      }
  }
}

void Ex_Remote_Builder(){
  if(SEND_BIT){
  switch(STATE_ENC){
    case ARBITRATION_LOSS_STATE:
      if (BUS_IDLE){
        STATE_ENC = SOF;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
      break; 
    case SOF:
      Ecount = 0;
      STATE_ENC = ID_A;
      Frame[Ecount] = '0';
      Serial.print("SOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case ID_A:        // 1 - 11 position
      if(!ARBITRATION_LOSS){
        if(Ecount < 11){
          Frame[Ecount] = ID[Ecount-1];
        }
        else {
          Frame[Ecount] = ID[Ecount-1];
          STATE_ENC = SRR;
        }
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("IDA: ");
        Serial.println(Frame[Ecount]);
        break;
    case SRR:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '1'; // 12 position
        STATE_ENC = IDE;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("RTR: ");
        Serial.println(Frame[Ecount]);
        break;
    case IDE:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '1';  // 13 position
        STATE_ENC = IDB;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("IDE: ");
        Serial.println(Frame[Ecount]);
        break;
    case IDB:
      if(!ARBITRATION_LOSS){
        if(Ecount < 31){
            Frame[Ecount] = idb[Ecount-14]; //14 - 31 position
          }
          else {
            Frame[Ecount] = idb[Ecount-14];
            STATE_ENC = RTR;
          }
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
          Serial.print("IDA: ");
          Serial.println(Frame[Ecount]);
          break;
    case RTR:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '1';   // 32 position
        STATE_ENC = R1;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
        Serial.print("R0: ");
        Serial.println(Frame[Ecount]);
        break;
    case R1:
      Frame[Ecount] = '0';   // 33 position
      STATE_ENC = R2;
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case R2:
      Frame[Ecount] = '0';   // 34 position
      STATE_ENC = DLC;
      Serial.print("R0: ");
      Serial.println(Frame[Ecount]);
      break;
    case DLC:
      if(Ecount < 38){
        Frame[Ecount] = dlc[Ecount-35];   // 35 - 38
      }
      else {
        Frame[Ecount] = dlc[Ecount-35];
        crc = MakeCRC(Frame);
        STATE_ENC = crce;
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case crce:
      if(Ecount < 53){      // 39 - 53 Position
        Frame[Ecount] = crc[Ecount - 39];
      }
      else {
        Frame[Ecount] = crc[Ecount - 39];
        STATE_ENC = CRC_DELIMITER;
      }
      Serial.print("CRC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC_DELIMITER:       //54 Position
      BS_FLAG = false;
      STATE_ENC = ACK_SLOT;
      Frame[Ecount] = '1';
      Serial.print("CRC_DELIMITER: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_SLOT:            //55 Position
      ACK_SLOT_FLAG = true;
      BS_FLAG = false;
      Frame[Ecount] = '1';  // First Encoder writes RECESSIVE bit
      if(ACK_CONFIRM){
        STATE_ENC = ACK_DELIMITER;
      }
      else{
        STATE_ENC = SOF;
      }
      Serial.print("ACK_SLOT: ");
      Serial.println(Frame[Ecount]);
      break;
    case ACK_DELIMITER:        //56 Position
      BS_FLAG = false;
      STATE_ENC = EoF;
      Frame[Ecount] = '1';
      Serial.println("ACK_DELIMITER");
      Serial.println(Frame[Ecount]);
      break;
    case EoF:
      BS_FLAG = false;
      if(Ecount < 63){        // 57-63
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = INTERFRAME_SPACING;
      }
      Serial.print("EOF: ");
      Serial.println(Frame[Ecount]);
      break;
    case INTERFRAME_SPACING:    //64 - 66 Position
      BS_FLAG = false;
      if(Ecount < 66){
        Frame[Ecount] = '1';
      }
      else {
        Frame[Ecount] = '1';
        STATE_ENC = WAIT;
      }
      Serial.print("INTERFRAME SPACING: ");
      Serial.println(Frame[Ecount]);
      break;
    case WAIT:
      Serial.println("FRAME END");
      STATE_ENC = WAIT;
      FRAME_START = true;
      //while(1);
      break;     
      }
      if(!ARBITRATION_LOSS){     
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
      }
  }
}

//ERROR FRAME CONSTRUCTOR

void Error_Builder(){
  if(SEND_BIT){
  switch(STATE_ENC){
    case ERROR_FLAG_STATE:
      if(Ecount < 5){
          Frame[Ecount] = '0';
        }
        else {
          Frame[Ecount] = '0';
          STATE_ENC = ERROR_DELIMITER;
        }
        BS_FLAG = false;
        Serial.print("ERROR_FLAG_STATE: ");
        Serial.println(Frame[Ecount]);
        break;
    case ERROR_DELIMITER:
    BS_FLAG = false;
      if(Ecount < 14){
            Frame[Ecount] = '1';
          }
          else {
            Frame[Ecount] = '1';
            STATE_ENC = WAIT;
          }
          Serial.print("ERROR_DELIMITER: ");
          Serial.println(Frame[Ecount]);
          break;
    case WAIT:
      Serial.println("FRAME END");
      STATE_ENC = WAIT;
      FRAME_START = true;
      //while(1);
      break;     
      } 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }  
}

//OVERLOAD FRAME CONSTRUCTOR

void Overload_Builder(){
  if(SEND_BIT){
  switch(STATE_ENC){
    case OVERLOAD_FLAG_STATE:
      if(Ecount < 5){
          Frame[Ecount] = '0';
        }
        else {
          Frame[Ecount] = '0';
          STATE_ENC = OVERLOAD_DELIMITER;
        }
        BS_FLAG = false;
        Serial.print("OVERLOAD FLAG: ");
        Serial.println(Frame[Ecount]);
        break;
    case OVERLOAD_DELIMITER:
      if(Ecount < 14){
            Frame[Ecount] = '1';
          }
          else {
            Frame[Ecount] = '1';
            STATE_ENC = WAIT;
          }
          Serial.print("OVERLOAD_DELIMITER: ");
          Serial.println(Frame[Ecount]);
          break;
    case WAIT:
      Serial.println("FRAME END");
      FRAME_START = true;
      //while(1);
      break;     
      } 
      BIT_TO_WRITE = Frame[Ecount];
      Ecount++; 
  }  
}      

void Frame_Builder(int FF,int FT,int DLC_L){
  BS_FLAG = true;
  if(!FRAME_START){
    if(FT == DATA_FRAME || FT == REMOTE_FRAME){
      STATE_ENC = SOF; //In DATA/REMOTE FRAME CASES
    }
    else if(FT == ERROR_FRAME){
      STATE_ENC = ERROR_FLAG_STATE; //In ERROR FRAME CASES
    }
    else if (FT == OVERLOAD_FRAME){
      STATE_ENC = OVERLOAD_FLAG_STATE; //In ERROR FRAME CASES
    }
    if(FF == BASE){
        Frame = (char*) calloc(47 + DLC_L*8,sizeof(char));  //BASE FRAME CREATION
        FRAME_START = true;
      }
      else if(FF == EXTENDED){
        Frame = (char*) calloc(67 + DLC_L*8,sizeof(char));  //EXTENDED FRAME CREATION
        FRAME_START = true;
      }
      else if(FT == ERROR_FRAME || OVERLOAD_FRAME){
        Frame = (char*) calloc(14,sizeof(char)); 
        FRAME_START = true;
      }
  }
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

    ////////***** ENCODER END ******////////



//Decoder BEGIN

void bit_stuffing_decoder(char Bit_Read){
 // Entradas:
 //   BSD_FLAG --> Indica se está num campo que pode ter bit stuffing ou não
  //  Bit_Read
  //  Sample_Point
  //  Saídas:
  //  BIT_TO_SAVE
 //   BSE_FLAG (Flag de Erro)
 //   CAPTURE (Sinal que manda o decoder capturar ou não o sinal)
    
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
            if(!BSD_FLAG){
              CAPTURE = true;
              STATE_BS_DEC = INACTIVE;
            }
            else{
              CAPTURE = false;
              if(Bit_Read == last_bit_dec){
                  BSE_FLAG = true;
                  STATE_BS_DEC = INACTIVE;
              }
              else{//bit stuffing gerado corretamente
                count_bs_decoder = 1;
                STATE_BS_DEC = COUNTING;
              }
            }
            last_bit_dec = Bit_Read;    
        break;
    }
}


 long int BinToDec(char bin[], int tam){
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

void check_id(unsigned int Extended_Flag,  char *ID_A_DECODER,  char *ID_B_DECODER,char *ID_A,  char *ID_B)
{
  bool ID_FLAG = true;
  int k = 0;

  for(k = 0; k < 11; k++)
  {
    if(ID_A_DECODER[k] != ID_A[k])
    {
      ID_FLAG  = false;
    }

  }

  if(Extended_Flag)
  {
    for(k = 0; k < 18; k++)
    {
      if(ID_B_DECODER[k] != ID_B[k])
      {
        ID_FLAG  = false;
      }

    }
  }

  if(ID_FLAG) 
  {
    Serial.println("ID Checked");
  }
  else
  {
    Serial.println("OUTRO ID");
  }

}

void print_frame(char RTR, char IDE, char *ID_A, char *ID_B,unsigned int Value_DLC, char *Data, char *CRC)
{
  long int Value;
  int i = 0;

  if(IDE == '1')
  {
    Serial.println("SRR = 1");
    Serial.println("IDE = 1");
    Serial.println("EXTENDED FRAME");

    Value = BinToDec(ID_A, 11);
    Serial.print("ID_A: 0x0");
    Serial.println(Value,HEX);

    Serial.print("ID_B: 0x0");
    BinToHex('0','0',ID_B[0],ID_B[1]);
    for(i = 0; i < 16; i += 4)
    {
      BinToHex(ID_B[i+2],ID_B[i+3],ID_B[i+4],ID_B[i+5]);
    }
    Serial.println("");
  }
  else if(IDE == '0')
  {
    Serial.println("IDE = 0");
    Serial.println("BASE FRAME");
    Value = BinToDec(ID_A, 11);
    Serial.print("ID_A: 0x0");
    Serial.println(Value,HEX);
  }

  if(RTR == '1')
  {
    Serial.println("RTR = 1");
    Serial.println("REMOTE FRAME");
    Serial.print("DLC: ");
    Serial.print(Value_DLC);

    Serial.println("");
    Serial.println("DATA: 0x00");
    
  }
  else if(RTR == '0')
  {
    Serial.println("RTR = 0");
    Serial.println("DATA FRAME");
    Serial.print("DATA: 0x");

    for(i = 0; i < ((Value_DLC*8));  i += 4)
    {
      BinToHex(Data[i],Data[i+1],Data[i+2],Data[i+3]);
     
    }
    Serial.println("");
  }
  
  Value = BinToDec(CRC, L_CRC);
  Serial.print("CRC:");
  Serial.println(CRC); //HEX ???
  
}

 void BinToHex(char bit1, char bit2, char bit3, char bit4) {
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


//UC_Decoder BEGIN
  
void UC_DECODER()
 {
  if(CAPTURE)
  {
    switch(STATE_DEC)
    {
      case BUS_IDLE:

        Serial.println("Bus_Idle");
        Vetor_Frame[0] = '0';
        count_frame = 1;
        
        if(OVERLOAD_FLAG_1)
        {
          Vetor_ID_A[count_decoder - 1] = BIT_TO_SAVE;
          //BSD_FLAG = false;
          STATE_DEC = ID_A;
          
        }
        else
        {
          if(BUS_IDLE_FLAG == true)
          {
            //Serial.print("if");
           // Serial.println(BIT_TO_SAVE);
            if(BIT_TO_SAVE == '0')
            {
              BUS_IDLE_FLAG = false;
              SoF_FLAG = true;
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
          else{
            Serial.println("else");
            Serial.print("count_decoder = ");
            Serial.println(count_decoder);
            Serial.print("BUS_IDLE_FLAG = ");
            Serial.println(BUS_IDLE_FLAG);
          }

        }
      break;

      case ID_A:
        
        SoF_FLAG = false;
        //Serial.print(BIT_TO_SAVE);
        Vetor_ID_A[count_decoder - 1] = BIT_TO_SAVE;
        aux_count += 1;
        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
        
        
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
        count_frame += 1;
        if(count_decoder == L_BIT)
        {
            Serial.println("RTR_SRR");
            if(BIT_TO_SAVE == '0')
            {
              count_decoder  = 0;
              Data_Flag = 1; //Data Frame
              BED_FLAG = true;
              STATE_DEC = IDE_0; 
              Serial.println("RTR = 0");
              Serial.println("Data frame");
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
        count_frame += 1;
        if(count_decoder == L_BIT && BED_FLAG == true)
        {
          
          Serial.println("IDE_0");
          if(BIT_TO_SAVE == '0')
          {
            Serial.println("IDE  = 0");
            Serial.println("Base frame Format");
            count_decoder  = 0;
            STATE_DEC = R0;
          }
          else
          {
            
            Serial.println("IDE = 1");
            Serial.println("Format Error"); 
            count_decoder  = 0;
            STATE_DEC = FORMAT_ERROR;
          }
        } 
      break;

      //States Extend

        case IDE_1:
          Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
          count_frame += 1;
          if(count_decoder == L_BIT)
          {
            Serial.println("IDE_1");
            if(BIT_TO_SAVE == '0')
            {
              Serial.println("RTR = 1");
              Serial.println("Remote Frame"); 
              Serial.println("IDE = 0");
              Serial.println("Base Frame Format");

              BED_FLAG = true;
              Remote_Flag = 1;
              count_decoder  = 0;
              STATE_DEC = R0;
            }
            else if(BIT_TO_SAVE == '1')
            {
              Serial.println("SRR = 1");
              Serial.println("IDE = 1");
              Serial.println("Extend Frame");
              Extended_Flag = 1;
              count_decoder  = 0;
              STATE_DEC = ID_B;
            }  
          }
      break;

      case ID_B:
        
        Vetor_ID_B[count_decoder -1] = BIT_TO_SAVE;
        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
        
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
        count_frame += 1;

        if(count_decoder == L_BIT)
        {
          Serial.println("RTR");
          BED_FLAG = true;

          if(BIT_TO_SAVE == '0')
          {
            Serial.println("Data frame");
            Serial.println("RTR = 0"); 
            Data_Flag = 1;
            count_decoder  = 0;
            STATE_DEC = R1R0;  
          }
          else
          {
            Serial.println("Remote frame");
            Serial.println("RTR = 1");
            Remote_Flag = 1;
            count_decoder  = 0;
            STATE_DEC = R1R0;  
          }     
        } 
      break;

      case R1R0:

        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;

        if(count_decoder == L_R1R0 && BED_FLAG == true)
        {
          count_frame += 2;
          Serial.println("R1R0");
          count_decoder  = 0;
          STATE_DEC = DLC;
        }
      break;       

      //States Extend
      case R0:

        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;
        count_frame += 1;

        if(count_decoder == L_BIT)
        {
          
          Serial.println("R0");

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
        Vetor_Frame[count_frame + count_decoder - 1] = BIT_TO_SAVE;

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
          Serial.println("ACK_SLOT");
          //BIT_TO_WRITE = '0';  
          ACK_FLAG = true;
          STATE_DEC = ACK_DELIMITER;
          count_decoder  = 0;
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
        } 
      break;

      case ACK_DELIMITER:
        if(count_decoder == L_BIT)
        {
          //CRC Checked = CRC_READ
          //Concatenar campos
          Resultado_CRC = MakeCRC(Vetor_Frame);
        
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
          else
          {
            Serial.println("ACK_DELIMITER FORMAT ERROR");
            count_decoder  = 0;
            STATE_DEC = FORMAT_ERROR;       
          }                
            
        } 
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
        else
        {
          count_decoder = 0;
          STATE_DEC = FORMAT_ERROR;
        }
      
      break;

      case INTERFRAME_SPACING:
        //Serial.print(BIT_TO_SAVE);
        
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
            SoF_FLAG = true;
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
//UC_Decoder END

//Decoder END


// Funções Auxiliares BEGIN
void check_ack(){
  if(CAN_RX == '0'){
    ACK_CONFIRM = true;
  }
  else{
    ACK_CONFIRM = false;
  }
}

void check_arbitration(){
  if(CAN_RX != CAN_TX){
    ARBITRATION_LOSS = true;
  }
  else{
    ARBITRATION_LOSS = false;
  }
}


//Funções Auxiliares END


//Loop BEGIN


void func_writing_point(){
    //Serial.println("Writing_point na função junto com BT");
    if(ACK_FLAG){//Flag do Decoder para indicar o envio de um bit recessivo de ACK_SLOT
      CAN_TX = '0';
      mySerial.write(CAN_TX);
      //Serial.print(mySerial.write(CAN_TX));
      //Serial.print(" CAN_TX == ");
      //Serial.println(CAN_TX);

    }
    else{
    Frame_Builder(FF,FT,DLC_L);
    //Frame_Printer(Frame,FF,FT,DLC_L);
    bit_stuffing_encoder();
    if(CAN_TX == '0'){
      mySerial.write(CAN_TX);
      //Serial.print(mySerial.write(CAN_TX));
      //Serial.print(" CAN_TX == ");
      //Serial.println(CAN_TX);
    }
    else if(CAN_TX == '1'){
      mySerial.write(CAN_TX);
      //Serial.print(mySerial.write(CAN_TX));
      //Serial.print(" CAN_TX == ");
      //Serial.println(CAN_TX);
    }
    //mySerial.write(CAN_TX);
    }
}


void func_sample_point(){
    if(ACK_SLOT_FLAG){
      check_ack();//Retorna ACK_SLOT_CONFIRM
    }
    bit_stuffing_decoder(CAN_RX);
    //Serial.print("BIT_TO_SAVE: ");
    //Serial.println(BIT_TO_SAVE);
   // Serial.print("capture: ");
    //Serial.println(CAPTURE);
    UC_DECODER();
    check_arbitration();//funçao q checa arbitração
}


//Bit_Timing_Module BEGIN

  //Edge Detector - Bit Timing
  void Edge_Detector(){
    if(BUS_IDLE && CAN_RX == '0'){//Hard_Sync
      HS_ISR();
    }
    else if (last_bit_bt == '1' && CAN_RX == '0'){//Soft_Sync
      SS_ISR();
    }
    last_bit_bt = CAN_RX;
  }

  void SS_ISR() {
    if(STATE_BT == SEG1){
      Soft_Sync = true;
      Ph_Error = min(count_bt,SJW);
    }  
    else if(STATE_BT == SEG2){
      Ph_Error = min(((L_SEG2 + 1)-count_bt),SJW);
      if(L_SEG2 - Ph_Error <= count_bt){
        SS_Flag = true;
      }
      Soft_Sync = true;
    }
  }

  void Inc_Count(){
    count_bt++;
    Plot_Tq = !Plot_Tq; 
  }

void UC_BT(/*SJW,CAN_RX,TQ,L_PROP,L_SYNC,L_SEG1,L_SEG2*/){
    Edge_Detector();
    Inc_Count();
    //print_state(); // função para debugar q n foi adicionada ao all
    Writing_Point = false;
    Sample_Point = false;

      switch(STATE_BT){
        case SYNC:
        //Serial.println("SYNC");
        Writing_Point = true;
        func_writing_point();
          if(count_bt >= L_SYNC){
              count_bt = 0; //0 ou 1 ?
              STATE_BT = SEG1;
          }
          break;
        
        case SEG1:
          //Serial.println("SEG1");
          if(count_bt == (L_SEG1 +Ph_Error)){
            STATE_BT = SEG2;
            Sample_Point = true;
            if(mySerial.available() > 0 ){
              CAN_RX = mySerial.read();//Capturar do barramento
              //Serial.print("CAN_RX = ");
              //Serial.println(CAN_RX);
              func_sample_point();
            }
            else{
              CAN_RX = '\0';
            }
            count_bt = 0;
            Ph_Error = 0;
          }
        
        break;
        
        case SEG2:
          //Serial.println("SEG2");
          if(count_bt == (L_SEG2 - Ph_Error) || SS_Flag){
            if(SS_Flag){
              STATE_BT = SEG1;
            }
            else{
              STATE_BT = SYNC;
            }
            SS_Flag = false;
            count_bt = 0;
            Ph_Error = 0;
          }
        break;
  }
    Hard_Sync = false;
    Soft_Sync = false;
}

void HS_ISR() {
  Hard_Sync = true;
  Timer1.start(); //reinicia timerone
  Timer1.attachInterrupt(UC_BT,TQ);//reinicia timerone
  count_bt = 0;
  STATE_BT = SYNC;//talvez antes da reinicialização do timerone, verificar
  Writing_Point = true;
  //Serial.println("Hard_Sync");
}


//Bit_Timing_Module END


//Setup BEGIN

void setup() {
  Serial.begin(9600);
  Timer1.initialize(TQ);
  Timer1.attachInterrupt(UC_BT);
  STATE_BT = SYNC;//State Bit Timing UC
  STATE_DEC = BUS_IDLE;//STATE DO DECODER
  STATE_BS_DEC = INACTIVE;//State do Bit Stuffing do Decoder
  STATE_BS_ENC = INACTIVE;//State do BS do Encoder
  STATE_SEND = FORMAT_SEND;
  count_bt = 0;//contador do Bit Timing
  attachInterrupt(0, HS_ISR, FALLING);
  attachInterrupt(1, SS_ISR, FALLING);

  //Comunicação Serial
  pinMode(CAN_RX_PIN,INPUT);
  pinMode(CAN_TX_PIN,OUTPUT);
  mySerial.begin(9600);
}

//Setup END


//Hex_to_Bin


void hex_to_bin(char* hex, char* bin){
   unsigned int i = 0; 

 /* Extract first digit and find binary of each hex digit */
    for(i=0; hex[i]!='\0'; i++){
        switch(hex[i])
        {
            case '0':
                    if(i!=0){
                    strcat(bin, "0000"); 
                    }
                break;
            case '1':
                    strcat(bin, "0001");
                break;
            case '2':
                    strcat(bin, "0010");
                  
                break;
            case '3':
                    strcat(bin, "0011");
                  
                break;
            case '4':
                    strcat(bin, "0100");
                  
                break;
            case '5':
                    strcat(bin, "0101");
                break;
            case '6':
                  strcat(bin, "0110");
                break;
            case '7':
                  strcat(bin, "0111");
                break;
            case '8':
                  strcat(bin, "1000");
                break;
            case '9':
                strcat(bin, "1001");
                break;
            case 'a':
            case 'A':
                strcat(bin, "1010");
                break;
            case 'b':
            case 'B':
                strcat(bin, "1011");
                break;
            case 'c':
            case 'C':
                strcat(bin, "1100");
                break;
            case 'd':
            case 'D':
                strcat(bin, "1101");
                break;
            case 'e':
            case 'E':
                strcat(bin, "1110");
                break;
            case 'f':
            case 'F':
                strcat(bin, "1111");
                break;
        }
    }
}

void send_frame(){
  switch (STATE_SEND){
    case FORMAT_SEND:
      Serial.println("Digite 'b' para base frame e 'e' para extended frame" );
      if(Serial.available() > 0 ){
        if(Serial.read() == 'b'){
          FF = BASE;
          STATE_SEND = TYPE_SEND;
        }
        else if(Serial.read() ==  'e'){
          FF = EXTENDED;
          STATE_SEND = TYPE_SEND;
        }
      }
    break;

    case TYPE_SEND:
      Serial.println("Digite 'd' para data frame e 'r' para remote frame" );
      if(Serial.available() > 0){
        if(Serial.read() == 'd'){
          FT = DATA_FRAME;
          STATE_SEND = ID_A_SEND;
        }
        else if(Serial.read() == 'r'){
          FT = REMOTE_FRAME;
          STATE_SEND = ID_A_SEND;
        }
      }
    break;

    case ID_A_SEND:
      Serial.println("Digite o ID_A do frame em Hexadecimal ");
      if(Serial.available() > 0 ){
        char ida_input [5] = "";//entrada em hexadecimal
        String input = Serial.readStringUntil('\n');
        input.toCharArray(ida_input,5);
        char aux [14] = "";
        hex_to_bin(ida_input,aux);
        for(int i = 0; i < 11; i++){
          ID[i] = aux[i+1];
        }
        if(FF == BASE){
          STATE_SEND = DLC_SEND;
        }
        else if(FF == EXTENDED){
          STATE_SEND = ID_B_SEND;
        }
      }
      break;
      
      case DLC_SEND:
      Serial.println("Digite o ID_B do frame em Hexadecimal ");
      if(Serial.available() > 0 ){
        char input_dlc = Serial.read();
        
        if(input_dlc > '8'){
          DLC_L = 8;
          dlc[0] = '1';
          dlc[1] = '0';
          dlc[2] = '0';
          dlc[3] = '0';
          STATE_SEND = DATA_SEND;
        }
        else if(input_dlc == '0'){
          DLC_L = 0;
          dlc[0] = '0';
          dlc[1] = '0';
          dlc[2] = '0';
          dlc[3] = '0';
          STATE_SEND = DATA_SEND;
        }
        else{
          //input.toCharArray(input_dlc,1);
          hex_to_bin(input_dlc,dlc);
          STATE_SEND = DATA_SEND; 
        }
      }

      break;
      
      case ID_B_SEND:
      Serial.println("Digite o ID_B do frame em Hexadecimal ");
      if(Serial.available() > 0 ){
        char idb_input [6] = "";//entrada em hexadecimal
        String input = Serial.readStringUntil('\n');
        input.toCharArray(idb_input,6);
        char aux [21] = "";
        hex_to_bin(idb_input,aux);
        for(int i = 0; i < 18; i++){
          idb[i] = aux[i+2];
        }
        STATE_SEND = DLC_SEND;

      break;

      case DATA_SEND:
      Serial.println("digite o valor do dado em Hexadecimal");
      if(Serial.available() > 0 ){
        char data_input [17] = "";//entrada em hexadecimal
        String input = Serial.readStringUntil('\n');
        input.toCharArray(data_input,16);
        hex_to_bin(data_input,data);
        FRAME_START = false;
        STATE_SEND = WAIT_SEND;
        //prints dos valores
        if(FF == BASE){
          Serial.println("Base Frame");
        }
        else if(FF = EXTENDED){
          Serial.println("Extended Frame");
        }

        if(FT == DATA_FRAME){
          Serial.println("Data Frame");
        }
        else if(FT = REMOTE_FRAME){
          Serial.println("Remote Frame");
        }

        Serial.print("ID_A: ");
        Serial.println(ID);
        if(FF == EXTENDED){
          Serial.print("ID_B: ");
          Serial.println(idb);
        }
        Serial.print("DlC: ");
        Serial.println(DLC_L);

        if(FT = DATA_FRAME){
          Serial.print("Data:");
          Serial.println(data);
        }
      }
      break;

      case WAIT_SEND:
          if(FRAME_START){
            STATE_SEND = FORMAT_SEND;
              ID [11] = "";         
              idb [18] = ""; 
              dlc[4] = "";  
              data [64] = "";
          }
      break;
  }

  //check_id(Extended_Flag,  ID_A_DECODER, ID_B_DECODER, ID_A, ID_B); //Confirmar se tá enviando para o mesmo ID
  //print_frame(RTR,IDE,ID_A,ID_B,Value_DLC,Data,CRC); //Chamar no Final de Cada Frame
}
}

void loop(){
  send_frame();
}

//Loop END