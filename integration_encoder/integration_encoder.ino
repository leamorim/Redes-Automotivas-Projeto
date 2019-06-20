  #include <TimerOne.h>
  #include <SoftwareSerial.h>
  
  bool BUS_IDLE_FLAG = true;
  char CAN_TX = '\0';
  char CAN_RX = '\0';
  bool GET_FRAME = true;
  String Frame_enc = "";

  enum end_dec_estados {BUS_IDLE = 0,SoF = 1,ID_A = 2,RTR_SRR = 3,IDE_0 = 4,R0 = 5, DLC = 6,
    DATA = 7, CRC_READ = 8,CRC_DELIMITER = 9, ACK_SLOT = 10, ACK_DELIMITER, EoF,
    INTERFRAME_SPACING,IDE_1, ID_B,RTR, R1R0, STATE_ERROR,
    FORMAT_ERROR, ACK_ERROR, CRC_ERROR, BIT_STUFFING_ERROR, STATE_BSD_FLAG1,
    BIT_ERROR , STATE_BED_FLAG1, OVERLOAD, WAIT , SOF, IDE, crce, SRR,R1,R2,
    IDB,ERROR_FLAG_STATE, ERROR_DELIMITER,
    OVERLOAD_DELIMITER, OVERLOAD_FLAG_STATE, ARBITRATION_LOSS_STATE} STATE_DEC, STATE_ENC;

  enum bs_estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_BS_ENC,STATE_BS_DEC;


//CRC_Module BEGIN

    char *MakeCRC(char *BitString) {
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


//Send_Frame Defines and functions BEGIN
  //Sinais de controle para construção de um frame
  #define DATA_FRAME 1
  #define REMOTE_FRAME 2
  #define ERROR_FRAME 3
  #define OVERLOAD_FRAME 4
  #define FRAME_TYPE REMOTE_FRAME

  #define BASE 0
  #define EXTENDED 1
  #define FRAME_FORMAT EXTENDED

  enum send_frame_states {FORMAT_SEND = 0, TYPE_SEND, ID_A_SEND, ID_B_SEND, DATA_SEND, WAIT_SEND, DLC_SEND} STATE_SEND;

  //Variáveis para construção do frame -- BEGIN
    char ID [13] = "";         //0x67E
    char idb [19] = "";        //0x3187A
    char dlc[5] = "";          //8 
    char data [65] = "";       //0xAAAAAAAAAAAAAAAA

    int DLC_L;
    int FF ; //FRAME FORMAT
    int FT ; //FRAME TYPE
    bool FRAME_START = true;
  //Variáveis para construção do frame -- END

  long int BinToDec(char bin[], int tam){
      unsigned int i;
      long int num = 0;

      for(i=0; i < tam; i++){
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

        if(Serial.available() > 0 ){
          char read_char = Serial.read();
          if(read_char == 'b'){
            FF = BASE;
            STATE_SEND = TYPE_SEND;
            Serial.println("Base ");
            Serial.println("Digite 'd' para data frame e 'r' para remote frame" );
          }
          if(read_char ==  'e'){
            FF = EXTENDED;
            STATE_SEND = TYPE_SEND;
            Serial.println("Extended ");
            Serial.println("Digite 'd' para data frame e 'r' para remote frame" );
          }
        }
      break;

      case TYPE_SEND:

        if(Serial.available() > 0){
          char read_char = Serial.read();
          if(read_char == 'd'){
            FT = DATA_FRAME;
            STATE_SEND = ID_A_SEND; 
            Serial.println("Data Frame");
            Serial.println("Digite o ID_A do frame de dados em Hexadecimal ");
          }
          else if(read_char == 'r'){
            FT = REMOTE_FRAME;
            STATE_SEND = ID_A_SEND;
            Serial.println("Remote Frame");     
            Serial.println("Digite o ID_A do frame remoto em Hexadecimal ");
          }
        }
      break;

      case ID_A_SEND:

        if(Serial.available() > 0 ){
          char ida_input [5] = "";//entrada em hexadecimal
          String input = Serial.readStringUntil('\n');
          Serial.println(input);
          input.toCharArray(ida_input,5);
          char aux [12] = "";
          hex_to_bin(ida_input,aux);
          Serial.print("ID_A == 0x");
          Serial.println(input);

          for(int i = 0; i < 12; i++){
            ID[i] = aux[i];
          }
   
          Serial.println(ID);
          if(FF == BASE){
            STATE_SEND = DLC_SEND;
            Serial.println("Digite DLC do frame em Número de bytes ");
          }
          else if(FF == EXTENDED){
            STATE_SEND = ID_B_SEND;
            Serial.println("Digite ID_B do frame em Hexadecimal");
          }
        }
        break;
        
        case DLC_SEND:
        if(Serial.available() > 0 ){
          char input_dlc [5] = "";//entrada em Decimal
          String dlc_input = Serial.readStringUntil('\n');
          dlc_input.toCharArray(input_dlc,5);
          hex_to_bin(input_dlc,dlc);
          DLC_L = BinToDec(dlc,4);
          Serial.print("DLC == ");
          Serial.println(dlc_input);

          if(DLC_L > 8){
            DLC_L = 8;
            dlc[0] = '1';
            dlc[1] = '0';
            dlc[2] = '0';
            dlc[3] = '0';
          } 

          if(FT == DATA_FRAME){
            STATE_SEND = DATA_SEND;
            Serial.println("digite o valor do dado em Hexadecimal");
          }
          else if(FT == REMOTE_FRAME){
            STATE_SEND = WAIT_SEND;
            GET_FRAME = false;
            FRAME_START = false;
          }
        }

        break;
        
        case ID_B_SEND:
        if(Serial.available() > 0 ){
          char idb_input [6] = "";//entrada em hexadecimal
          String input = Serial.readStringUntil('\n');
          input.toCharArray(idb_input,6);
          char aux [24] = "";
          hex_to_bin(idb_input,aux);
          Serial.print("ID_B: 0x");
          Serial.println(input);

          for(int i = 0; i < 18; i++){
            idb[i] = aux[i+2];
          }
          STATE_SEND = DLC_SEND;
          Serial.println("digite o valor do DLC em número de bytes");
        }
        break;

        case DATA_SEND:
        if(Serial.available() > 0 ){
          char data_input [17] = "";//entrada em hexadecimal
          String input = Serial.readStringUntil('\n');
          input.toCharArray(data_input,17);
          hex_to_bin(data_input,data);
          Serial.print("Data: 0x");
          Serial.println(input);

          STATE_SEND = WAIT_SEND;
          FRAME_START = false;
          GET_FRAME = false;
        }
        break;

        case WAIT_SEND:
            if(GET_FRAME){
              STATE_SEND = FORMAT_SEND;
              Serial.println("Digite 'b' para base frame e 'e' para extended frame" );
              ID [0] = '\0';         
              idb [0] = '\0'; 
              dlc [0] = '\0';  
              data [0] = '\0';
              DLC_L = 0;
            }
        break;
    }
  }
//Send_Frame Defines and functions END


//Encoder BEGIN
    //Encoder BEGIN
    bool ARBITRATION_LOSS =  false;//Indica a perca de Arbitração, este valor é entrada do Frame builder e saída do BS
    bool ACK_SLOT_FLAG = false;//Sinal enviado do Frame_Builder para o BS para indicar que o campo atual é o de ACK_SLOT
    bool ACK_CONFIRM =  true;//Sinal enviado do BS para o Frame_Builder para indicar que o ACK foi recebido com sucesso

    int Ecount = 0;
    char *crc;
    char *Frame = NULL;
    String printvec = ""; //printa o frame montado após o módulo de Bit Stuffing

    //Variáveis BS Encoder

    unsigned int count_encoder = 0;
    char last_bit_enc;
    bool SEND_BIT = true;
    bool BS_FLAG = false;
    char BIT_TO_WRITE;

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

  //Base Frame
  void Data_Builder(int DLC_L){
    if(SEND_BIT){
    switch(STATE_ENC){
      case ARBITRATION_LOSS_STATE:
        if (BUS_IDLE_FLAG){
          STATE_ENC = SOF;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
        break; 
      case SOF:
        Ecount = 0;
        BS_FLAG = true;
        Frame[Ecount] = '0';
        Serial.print("SOF ");
        STATE_ENC = ID_A;
        
        break;
      case ID_A:
        if(!ARBITRATION_LOSS){
          if(Ecount < 11){
            Frame[Ecount] = ID[Ecount];
          }
          else {
            Frame[Ecount] = ID[Ecount];   
            STATE_ENC = RTR;
          }
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
    //     Serial.print("IDA: ");
    //     Serial.println(Frame[Ecount]);
          break;
      case RTR:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '0'; // 12 position
          STATE_ENC = IDE;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
    //      Serial.print("RTR: ");
    //      Serial.println(Frame[Ecount]);
          break;
      case IDE:
        Frame[Ecount] = '0';  // 13 position
        STATE_ENC = R0;
      //  Serial.print("IDE: ");
      //  Serial.println(Frame[Ecount]);
        break;
      case R0:
        Frame[Ecount] = '0';   // 14 position
        STATE_ENC = DLC;
      //  Serial.print("R0: ");
      //  Serial.println(Frame[Ecount]);
        break;
      case DLC:
        if(Ecount < 18){
          Frame[Ecount] = dlc[Ecount-15];   // 15 - 18
        }
        else {
          Frame[Ecount] = dlc[Ecount-15];
          STATE_ENC = DATA;
        }

      //  Serial.print("DLC: ");
      //  Serial.println(Frame[Ecount]);
        break;
      case DATA:
        BS_FLAG = true;
        if((Ecount < 18 + DLC_L*8) && (DLC_L != 0)){
          Frame[Ecount] = data[Ecount-19];
        }
        else {
          Frame[Ecount] = data[Ecount-19];
          crc = MakeCRC(Frame);
    //     Serial.println("CRC" );
    //     Serial.print(crc);
          STATE_ENC = crce;
        }
    //    Serial.print("DATA: ");
    //    Serial.println(Frame[Ecount]);
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
    //    Serial.print("CRC: ");
    //    Serial.println(Frame[Ecount]);
        break;
      case CRC_DELIMITER:
        BS_FLAG = false;
        STATE_ENC = ACK_SLOT;
        Frame[Ecount] = '1';
    //   Serial.print("CRC_DELIMITER: ");
    //   Serial.println(Frame[Ecount]);
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
      // Serial.print("ACK_SLOT: ");
        //Serial.println(Frame[Ecount]);
        break;
      case ACK_DELIMITER:
        BS_FLAG = false;
        STATE_ENC = EoF;
        Frame[Ecount] = '1';
      //  Serial.println("ACK_DELIMITER");
      //  Serial.println(Frame[Ecount]);
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
    //   Serial.println(Frame[Ecount]);
        break;
      case INTERFRAME_SPACING:
        BS_FLAG = false;
        if(Ecount < (46 + (DLC_L*8))){
          Frame[Ecount] = '1';
        }
        else {
          Frame[Ecount] = '1';
          STATE_ENC = WAIT;
          Serial.print("FRAME ENC: ");
          Serial.println(Frame_enc);

          Serial.println("FRAME PRINTER: ");
          Frame_Printer(Frame,FF,FT,DLC_L);
          free(Frame);
          //Frame = NULL;
          Serial.println("FRAME END");
        }
        Serial.print("INTERFRAME SPACING: ");
    //   Serial.println(Frame[Ecount]);
        break;
      case WAIT:
        Serial.println("FRAME END");
        STATE_ENC = WAIT;
        GET_FRAME = true;
        break;
      }
        if(!ARBITRATION_LOSS){       
          BIT_TO_WRITE = Frame[Ecount];  
        // Serial.print("bit_to_w:");
        // Serial.print(BIT_TO_WRITE);
          //Serial.print(" frame[ec] ");
          //Serial.println(Frame[Ecount]);
          Ecount++; 
        }
    }
        
  }

  void Remote_Builder(){
    if(SEND_BIT){
    switch(STATE_ENC){
      case ARBITRATION_LOSS_STATE:
        if(BUS_IDLE_FLAG){
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
        //Serial.println(Frame[Ecount]);
        break;
      case ID_A:
        if(!ARBITRATION_LOSS){
          if(Ecount < 11){
            Frame[Ecount] = ID[Ecount];
          }
          else {
            Frame[Ecount] = ID[Ecount];
            STATE_ENC = RTR;
          }
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
     //       Serial.print("IDA: ");
          //Serial.println(Frame[Ecount]);
          break;
      case RTR:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '1'; // 12 position
          STATE_ENC = IDE;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
           //Serial.print("RTR: ");
           //Serial.println(Frame[Ecount]);
          break;
      case IDE:
        Frame[Ecount] = '0';  // 13 position
        STATE_ENC = R0;
        //Serial.print("IDE: ");
        //Serial.println(Frame[Ecount]);
        break;
      case R0:
        Frame[Ecount] = '0';   // 14 position
        STATE_ENC = DLC;
          //Serial.print("R0: ");
          //Serial.println(Frame[Ecount]);
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
          //Serial.print("DLC: ");
          //Serial.println(Frame[Ecount]);
        break;
      case crce:
        if(Ecount < 33){
          Frame[Ecount] = crc[Ecount - 19];
        }
        else {
          Frame[Ecount] = crc[Ecount - 19];
          STATE_ENC = CRC_DELIMITER;
        }
          //Serial.print("CRC: ");
          //Serial.println(Frame[Ecount]);
        break;
      case CRC_DELIMITER:
        BS_FLAG = false;
        STATE_ENC = ACK_SLOT;
        Frame[Ecount] = '1';
          //Serial.print("CRC_DELIMITER: ");
          //Serial.println(Frame[Ecount]);
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
         //Serial.print("ACK_SLOT: ");
         //Serial.println(Frame[Ecount]);
        break;
      case ACK_DELIMITER:
        BS_FLAG = false;
        STATE_ENC = EoF;
        Frame[Ecount] = '1';
         //Serial.println("ACK_DELIMITER");
         //Serial.println(Frame[Ecount]);
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
        //Serial.print("EOF: ");
        //Serial.println(Frame[Ecount]);
        break;
      case INTERFRAME_SPACING:
        BS_FLAG = false;
        if(Ecount < 46){
          Frame[Ecount] = '1';
        }
        else {
          Frame[Ecount] = '1';
          STATE_ENC = WAIT;
          Serial.print("FRAME: ");  
          Serial.println(Frame);
          Serial.println("FRAME PRINTER: ");
          Frame_Printer(Frame,FF,FT,DLC_L);
          free(Frame);
          Serial.println("FRAME END");
        }
          //Serial.print("INTERFRAME SPACING: ");
          //Serial.println(Frame[Ecount]);
        break;
      case WAIT:
        STATE_ENC = WAIT;
        GET_FRAME = true;
        break;     
      }
        if(!ARBITRATION_LOSS){    
        BIT_TO_WRITE = Frame[Ecount];
        Ecount++; 
        }
    }
  }

  //Extended Frame
  void Ex_Data_Builder(int DLC_L){
    if(SEND_BIT){
    switch(STATE_ENC){
      case ARBITRATION_LOSS_STATE:
        if (BUS_IDLE_FLAG){
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
    //    Serial.println(Frame[Ecount]);
        break;
      case ID_A:        // 1 - 11 position
        if(!ARBITRATION_LOSS){
            if(Ecount < 11){
              Frame[Ecount] = ID[Ecount];
            }
            else {
              Frame[Ecount] = ID[Ecount];
              STATE_ENC = SRR;
            }
          }
          else{
            STATE_ENC = ARBITRATION_LOSS_STATE;
          }
    //     Serial.print("IDA: ");
    //     Serial.println(Frame[Ecount]);
          break;
      case SRR:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '1'; // 12 position
          STATE_ENC = IDE;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
    //     Serial.print("RTR: ");
    //     Serial.println(Frame[Ecount]);
          break;
      case IDE:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '1';  // 13 position
          STATE_ENC = IDB;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
    //     Serial.print("IDE: ");
    //     Serial.println(Frame[Ecount]);
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
    //       Serial.print("IDB: ");
    //       Serial.println(Frame[Ecount]);
            break;
            
      case RTR:
      if(!ARBITRATION_LOSS){
        Frame[Ecount] = '0';   // 32 position
        STATE_ENC = R1;
      }
      else{
        STATE_ENC = ARBITRATION_LOSS_STATE;
      }
    //   Serial.print("R0: ");
    //   Serial.println(Frame[Ecount]);
        break;
      case R1:
        Frame[Ecount] = '0';   // 33 position
        STATE_ENC = R2;
    //     Serial.print("R0: ");
    //     Serial.println(Frame[Ecount]);
        break;
      case R2:
        Frame[Ecount] = '0';   // 34 position
        STATE_ENC = DLC;
    //    Serial.print("R0: ");
    //    Serial.println(Frame[Ecount]);
        break;
      case DLC:
        if(Ecount < 38){
          Frame[Ecount] = dlc[Ecount-35];   // 35 - 38
        }
        else {
          Frame[Ecount] = dlc[Ecount-35];
          STATE_ENC = DATA;
        }
    //    Serial.print("DLC: ");
    //    Serial.println(Frame[Ecount]);
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
    //      Serial.print("DATA: ");
    //      Serial.println(Frame[Ecount]);
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
    //    Serial.print("CRC: ");
    //    Serial.println(Frame[Ecount]);
        break;
      case CRC_DELIMITER:       //54 (+DLC_L) Position
        STATE_ENC = ACK_SLOT;
        BS_FLAG = false;
        Frame[Ecount] = '1';
    //     Serial.print("CRC_DELIMITER: ");
    //    Serial.println(Frame[Ecount]);
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
    //    Serial.print("ACK_SLOT: ");
    //    Serial.println(Frame[Ecount]);
        break;
      case ACK_DELIMITER:        //56 (+DLC_L) Position
        STATE_ENC = EoF;
        Frame[Ecount] = '1';
        BS_FLAG = false;      
    //   Serial.println("ACK_DELIMITER");
      //  Serial.println(Frame[Ecount]);
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
    //    Serial.print("EOF: ");
    //    Serial.println(Frame[Ecount]);
        break;
      case INTERFRAME_SPACING:    //64 (+DLC_L) - 66 (+DLC_L) Position
        Serial.println("INTERFRAME");
        BS_FLAG = false;
        if(Ecount < 66 + (DLC_L*8)){
          Frame[Ecount] = '1';
        }
        else {
          Frame[Ecount] = '1';
          STATE_ENC = WAIT;
          Serial.print("FRAME: ");        
          Serial.println(Frame);
          free(Frame);
          Serial.println("FRAME END");
        }
        break;
      case WAIT:
        STATE_ENC = WAIT;
        GET_FRAME = true;
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
        if (BUS_IDLE_FLAG){
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
            Frame[Ecount] = ID[Ecount];
          }
          else {
            Frame[Ecount] = ID[Ecount];
            STATE_ENC = SRR;
          }
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
        //  Serial.print("IDA: ");
        //  Serial.println(Frame[Ecount]);
          break;
      case SRR:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '1'; // 12 position
          STATE_ENC = IDE;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
         // Serial.print("RTR: ");
         // Serial.println(Frame[Ecount]);
          break;
      case IDE:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '1';  // 13 position
          STATE_ENC = IDB;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
          //Serial.print("IDE: ");
          //Serial.println(Frame[Ecount]);
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
            //Serial.print("IDA: ");
            //Serial.println(Frame[Ecount]);
            break;
      case RTR:
        if(!ARBITRATION_LOSS){
          Frame[Ecount] = '1';   // 32 position
          STATE_ENC = R1;
        }
        else{
          STATE_ENC = ARBITRATION_LOSS_STATE;
        }
          //Serial.print("R0: ");
          //Serial.println(Frame[Ecount]);
          break;
      case R1:
        Frame[Ecount] = '0';   // 33 position
        STATE_ENC = R2;
        //Serial.print("R0: ");
        //Serial.println(Frame[Ecount]);
        break;
      case R2:
        Frame[Ecount] = '0';   // 34 position
        STATE_ENC = DLC;
        //Serial.print("R0: ");
        //Serial.println(Frame[Ecount]);
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

        //Serial.print("DLC: ");
        //Serial.println(Frame[Ecount]);
        break;
      case crce:
        if(Ecount < 53){      // 39 - 53 Position
          Frame[Ecount] = crc[Ecount - 39];
        }
        else {
          Frame[Ecount] = crc[Ecount - 39];
          STATE_ENC = CRC_DELIMITER;
        }
        //Serial.print("CRC: ");
        //Serial.println(Frame[Ecount]);
        break;
      case CRC_DELIMITER:       //54 Position
        BS_FLAG = false;
        STATE_ENC = ACK_SLOT;
        Frame[Ecount] = '1';
        //Serial.print("CRC_DELIMITER: ");
        //Serial.println(Frame[Ecount]);
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
        //Serial.print("ACK_SLOT: ");
        //Serial.println(Frame[Ecount]);
        break;
      case ACK_DELIMITER:        //56 Position
        BS_FLAG = false;
        STATE_ENC = EoF;
        Frame[Ecount] = '1';
        //Serial.println("ACK_DELIMITER");
        //Serial.println(Frame[Ecount]);
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
        //Serial.print("EOF: ");
        //Serial.println(Frame[Ecount]);
        break;
      case INTERFRAME_SPACING:    //64 - 66 Position
        BS_FLAG = false;
        if(Ecount < 66){
          Frame[Ecount] = '1';
        }
        else {
          Frame[Ecount] = '1';
          STATE_ENC = WAIT;
          Serial.print("FRAME: ");  
          Serial.println(Frame);
          free(Frame);
          Serial.println("FRAME END");
        }
        break;
      case WAIT:
        STATE_ENC = WAIT;
        GET_FRAME = true;
        break;     
        }
        if(!ARBITRATION_LOSS){     
        BIT_TO_WRITE = Frame[Ecount];
        Ecount++; 
        }
    }
  }

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
          //Serial.print("ERROR_FLAG_STATE: ");
          //Serial.println(Frame[Ecount]);
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
            free(Frame);
            Serial.println("FRAME END");
            //Serial.print("ERROR_DELIMITER: ");
            //Serial.println(Frame[Ecount]);
            break;
      case WAIT:

        STATE_ENC = WAIT;
        GET_FRAME = true;
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
          //Serial.print("OVERLOAD FLAG: ");
          //Serial.println(Frame[Ecount]);
          break;
      case OVERLOAD_DELIMITER:
        if(Ecount < 14){
              Frame[Ecount] = '1';
            }
            else {
              Frame[Ecount] = '1';
              STATE_ENC = WAIT;
            }
            free(Frame);
            Serial.println("FRAME END");
            //Serial.print("OVERLOAD_DELIMITER: ");
            //Serial.println(Frame[Ecount]);
            break;
      case WAIT:
        GET_FRAME = true;
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

//Encoder END

//Bit_Timing_Module BEGIN

    #define CAN_RX_PIN 7
    #define CAN_TX_PIN 8
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



    
    void func_writing_point(){
        if(!GET_FRAME){
          Frame_Builder(FF,FT,DLC_L);
          //Frame_Printer(Frame,FF,FT,DLC_L);
          bit_stuffing_encoder();
          
        if(CAN_TX == '0'){//aqui que vai escrever no barramento, fazer
          mySerial.write(CAN_TX);
          //Serial.print(mySerial.write(CAN_TX));
         // Serial.print(" CAN_TX == ");
         // Serial.println(CAN_TX);
         Frame_enc.concat(CAN_TX);
        }
        else if(CAN_TX == '1'){
          mySerial.write(CAN_TX);
          //Serial.print(mySerial.write(CAN_TX));
          //Serial.print(" CAN_TX == ");
          //Serial.println(CAN_TX);
          Frame_enc.concat(CAN_TX);
        }   

        }
    }

    //Edge Detector - Bit Timing
    void Edge_Detector(){
        if(BUS_IDLE_FLAG && CAN_RX == '0'){//Hard_Sync
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
             /*  if(mySerial.available() > 0 ){
                 CAN_RX = mySerial.read();//Capturar do barramento
                  Serial.print("CAN_RX = ");
                  Serial.println(CAN_RX);
                    func_sample_point();
                }
                else{
                  CAN_RX = '\0';
                }
               */
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
    Serial.begin(115200);
    Timer1.initialize(TQ);
    Timer1.attachInterrupt(UC_BT);
    STATE_BT = SYNC;//State Bit Timing UC
    //STATE_DEC = BUS_IDLE;//STATE DO DECODER
  // STATE_BS_DEC = INACTIVE;//State do Bit Stuffing do Decoder
    STATE_BS_ENC = INACTIVE;//State do BS do Encoder
    STATE_SEND = FORMAT_SEND;
    count_bt = 0;//contador do Bit Timing
    //  attachInterrupt(0, HS_ISR, FALLING);
    //  attachInterrupt(1, SS_ISR, FALLING);
    pinMode(CAN_TX_PIN,OUTPUT);
    pinMode(CAN_RX_PIN,INPUT);

    //Comunicação Serial
    //  pinMode(CAN_RX_PIN,INPUT);
    //  pinMode(CAN_TX_PIN,OUTPUT);
     mySerial.begin(115200);
    Serial.println("Digite 'b' para base frame e 'e' para extended frame" );
  }
//Setup END


void loop(){
  send_frame();
}
