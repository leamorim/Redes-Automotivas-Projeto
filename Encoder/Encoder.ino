#include <TimerOne.h>
#include <stdlib.h>

//STATES

#define SOF 0
#define ID_A 1
#define RTR 2
#define IDE 3
#define R0 4
#define DLC 5
#define DATA 6
#define CRC 7
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
char ID[11] = {'9','7','7','7','7','8','7','7','7','7','9'};
char dlc[4] = {'D','L','C','E'};
char crc[15] = {'9','3','3','3','3','3','3','8','3','3','3','3','3','3','9'};
char data[8] = {'0','1','1','1','1','1','1','0'};
int DLC_L = 1;
//char *data =  malloc(sizeof(char) * (DLC_L*8));
//char *Frame = (char*) calloc(55,sizeof(char));
char *Frame = (char*) calloc(47 + DLC_L*8,sizeof(char));
char *printvec;

void setup() {
  Serial.begin(9600);
  STATE = SOF;
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
  Serial.print("Data Builder\n");
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
        STATE = CRC;
        Ecount--;
        Serial.print("COUNT DATA:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }
      Serial.print("DATA: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC:
      if((Ecount < 34 + DLC_L*8)){
        Frame[Ecount] = crc[Ecount - 19 -(DLC_L*8)];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = CRC_DELIMITER;
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
}

void Remote_Builder(){
  Serial.print("Remote Builder\n");
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
        STATE = CRC;
        Ecount--;
        Serial.print("COUNT DLC:  ");
        Serial.println(Ecount);
        //0 | 1 2 3 4 5 6 7  8 9 10 11 | 12 | 13 | 14 |
      }

      Serial.print("DLC: ");
      Serial.println(Frame[Ecount]);
      break;
    case CRC:
      if(Ecount < 34){
        Frame[Ecount] = crc[Ecount - 19];
        Serial.print("COUNT CRC:  ");
        Serial.println(Ecount);
      }
      else {
        STATE = CRC_DELIMITER;
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
  Ecount++;
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
