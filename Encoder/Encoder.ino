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
#define EOF 11
#define INTERFRAME_SPACING 12
#define WAIT 13

//CONTROL SIGNALS

#define TQ 1000000
#define DATA_FRAME 0
#define REMOTE_FRAME 1
#define FF 0 
#define FT 1
#define FRAME_TYPE REMOTE_FRAME



//FF = 0 -> Base Format / FF = 1 -> Extended Format
//FT = 0 -> Data Frame / FT = 1 -> Remote Frame / FT = 2 -> Error Frame / FT = 3 -> Overload Frame

volatile byte STATE;
int count = 0;
char ID[11] = {'7','1','1','1','7','0','1','1','1','1','7'};
char dlc[4] = {'D','L','C','E'};
char crc[15] = {'3','3','3','3','3','3','3','8','3','3','3','3','3','3','9'};
char data[8] = {'9','1','1','1','1','1','1','9'};
int DLC_L = 1;
//char *data =  malloc(sizeof(char) * (DLC_L*8));
char *Frame = malloc(sizeof(char) * 55);
char *printvec;

void setup() {
  Serial.begin(9600);
  STATE = SOF;
  //Timer1.initialize(TQ);   //(PARAMETRO EM MICROSEGUNDOS)
  //Timer1.attachInterrupt(Inc_Count);

}

void Frame_Printer(char*v){
  Serial.println("FRAME:");
  for(int i = 0;i <55;i++){
    Serial.print(v[i]);
  }
  Serial.println();
}

void Inc_Count(){
  count++;
}

void Data_Builder(){
  Serial.print("Data Builder\n");
  switch(STATE){
    case SOF:
      if(count == 1){
        STATE = ID_A;
        count = 0; 
        break;
      }
       Frame[0] = '0';
    Serial.print("SOF\n");
    case ID_A:
      if(count == 11){
        STATE = RTR;
        count = 0; 
        break;
      }
      Frame[count+1] = ID[count];
    case RTR:
      Frame[12] = '0';
      if(count == 1){
        STATE = IDE;
        count = 0; 
        break;
      }
    case IDE:
      Frame[13] = '0';
      if(count == 1){
        STATE = R0;
        count = 0;
        break; 
      }
    case R0:
      Frame[14] = '0';
      if(count == 1){
        STATE = DLC;
        count = 0; 
        break;
      }
    case DLC:
      if(count == 4){
        STATE = DATA;
        count = 0; 
        break;
      }
      Frame[count + 15] = dlc[count];
    case DATA:
      if(count == (DLC_L*8)){
        STATE = CRC;
        count = 0; 
        break;
      }
      Frame[count +19] = data[count];
    case CRC:
      if(count == 16){
        STATE = CRC_DELIMITER;
        count = 0; 
        break;
      }
      Frame[count + 19 + (DLC_L*8)] = crc[count];
      //break;
    case CRC_DELIMITER:
      if(count == 1){
        STATE = ACK_SLOT;
        count = 0; 
        break;
      }
      Frame[count + 35 + (DLC_L*8)] = '1';
    case ACK_SLOT:
      if(count == 1){
        STATE = ACK_DELIMITER;
        count = 0; 
        break;
      }
      Frame[count + 36 + (DLC_L*8)] = '1';  // First Encoder writes RECESSIVE bit
    case ACK_DELIMITER:
      if(count == 1){
        STATE = EOF;
        count = 0; 
        break;
      }
      Frame[count + 37 + (DLC_L*8)] = '1';
    case EOF:
      if(count == 7){
        STATE = INTERFRAME_SPACING;
        count = 0;
        break; 
      }
      Frame[count + 44 + (DLC_L*8)] = '1';
    case INTERFRAME_SPACING:
      if(count == 4){
        count = 0; 
        STATE = SOF;
        break;
      }
      Frame[count + 44 + (DLC_L*8)] = '1';
    case WAIT:
      break;     
      } 
  }

void Remote_Builder(){
  Serial.print("Remote Builder\n");
  switch(STATE){
    case SOF:
      if(count == 1){
        STATE = ID_A;
        count = 0; 
        break;
      }
       Frame[0] = '0';
       Serial.flush();
       Serial.print("SOF\n");
    case ID_A:
      if(count == 11){
        STATE = RTR;
        count = 0; 
        break;
      } // 0 - 1234567891011
      Frame[count+1] = ID[count];
    case RTR:
      Frame[12] = '0';
      if(count == 1){
        STATE = IDE;
        count = 0; 
        break;
      }
    case IDE:
      Frame[13] = '0';
      if(count == 1){
        STATE = R0;
        count = 0;
        break; 
      }
    case R0:
      Frame[14] = '0';
      if(count == 1){
        STATE = DLC;
        count = 0; 
        break;
      }
    case DLC:
      if(count == 4){
        STATE = DATA;
        count = 0; 
        break;
      }
      Frame[count + 15] = dlc[count];
    case CRC:
      if(count == 16){
        STATE = CRC_DELIMITER;
        count = 0; 
        break;
      }
      Frame[count +19] = crc[count];
      //break;
    case CRC_DELIMITER:
      if(count == 1){
        STATE = ACK_SLOT;
        count = 0; 
        break;
      }
      Frame[count + 35] = '1';
    case ACK_SLOT:
      if(count == 1){
        STATE = ACK_DELIMITER;
        count = 0; 
        break;
      }
      Frame[count + 36] = '1';  // First Encoder writes RECESSIVE bit
    case ACK_DELIMITER:
      if(count == 1){
        STATE = EOF;
        count = 0; 
        break;
      }
      Frame[count + 37] = '1';
    case EOF:
      if(count == 7){
        STATE = INTERFRAME_SPACING;
        count = 0;
        break; 
      }
      Frame[count + 44 + DLC_L*8] = '1';
    case INTERFRAME_SPACING:
      if(count == 4){
        count = 0; 
        STATE = SOF;
        break;
      }
      Frame[count + 44 + (DLC_L*8)] = '1';
    case WAIT:
      break;     
      } 
  }
void Error_Builder();
void Overload_Builder();

void Frame_Builder(){
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
  Frame_Builder();
  count++;
  Serial.flush();
  Frame_Printer(Frame);
  Serial.flush();
  //delay(500);
}
  // put your main code here, to run repeatedly:
