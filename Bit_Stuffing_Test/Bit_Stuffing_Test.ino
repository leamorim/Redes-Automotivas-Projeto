enum estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_ENC,STATE_DEC;

unsigned int count_encoder = 0;
unsigned int count_decoder = 0;
char last_bit_enc, last_bit_dec;


void setup(){
    Serial.begin(115200);
    STATE_DEC = INACTIVE;
    STATE_ENC = INACTIVE;
}

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


bool bit_stuffing_decoder(bool BSD_FLAG, char Bit_Read){
 /* Entradas:
    BSD_FLAG --> Indica se está num campo que pode ter bit stuffing ou não
    Bit_Read
    Sample_Point

    Saídas:
    BIT_TO_SAVE
    BSE_FLAG (Flag de Erro)
    CAPTURE (Sinal que manda o decoder capturar ou não o sinal)
 */   


//Assigns são feitos para estas saídas em TODOS os estados
    char BIT_TO_SAVE;
    bool CAPTURE,BSE_FLAG; 

    switch (STATE_DEC){
    case INACTIVE:
        if(BSD_FLAG){
            STATE_DEC = COUNTING;
            count_decoder = 1;
        }
        else{
            STATE_DEC = INACTIVE;           
        }
        CAPTURE = true;
        BIT_TO_SAVE = Bit_Read;
        BSE_FLAG = false;
        last_bit_dec = Bit_Read;
        
        break;
        
    case COUNTING:
        if(!BSD_FLAG){
            STATE_DEC = INACTIVE;
            count_decoder = 0;
        }
        else{
            if(Bit_Read != last_bit_dec){
                count_decoder = 1;
            }
            else{
                count_decoder++;
                if(count_decoder == 5){//está no 5º bit lido
                  STATE_DEC = BIT_STUFFED;//estado q verifica o 6º bit lido
                } 
            }
        }

        BIT_TO_SAVE = Bit_Read;
        last_bit_dec = Bit_Read; 
        CAPTURE = true;
        BSE_FLAG = false;   
        
        break;    

        case BIT_STUFFED:
            count_decoder++;
            CAPTURE = false;            
            if(Bit_Read != last_bit_dec){//bit stuffing foi gerado corretamente e deve ser retirado     
                  count_decoder = 1;
                  if(BSD_FLAG){              
                      STATE_DEC = COUNTING;
                  }
              }
              else{//erro de bit stuffing
                  BSE_FLAG = true;
                  STATE_DEC = INACTIVE;
              }             
              last_bit_dec = Bit_Read;    
        break;
    }

    if(CAPTURE){
      Serial.print(BIT_TO_SAVE);
    }

/*
    Serial.println("ordem: STATE_DEC/Bit_Read/BIT_TO_SAVE/count_decoder/CAPTURE/last_bit/BSE_FLAG");
    Serial.print(STATE_DEC);
    Serial.print("/");
    Serial.print(Bit_Read);
    Serial.print("/");
    Serial.print(BIT_TO_SAVE);
    Serial.print("/");
    Serial.print(count_decoder);
    Serial.print("/");
    Serial.print(CAPTURE);
    Serial.print("/");
    Serial.print(last_bit_dec);
    Serial.print("/");
    Serial.print(BSE_FLAG);
    Serial.println("/");
*/
    return CAPTURE;
}

char teste1 [] = {'0','0','0','0','0','0','1','1','1','1','1','1'};//saída: 0 0 0 0 0 1 0 1 1 1 1 1 0 1
                                                                        //  0 0 0 0 0 1 0 1 1 1 1 1 0 1


char teste2 [] = {'0','0','0','0','0','1','0','1','1','1','1','1','0','1'};
            

void loop(){
    bool retorno = true;
    for(int i = 0; i < sizeof(teste1);){//14 eh o tamanho de teste1
        retorno = bit_stuffing_encoder(true,teste1[i]);
        if(retorno == true){
            i++;
        }
    }
    Serial.println("");
    Serial.println("FIM ENCODER");
    //teste do bit_stuffing_decoder
    
    for(int i = 0; i < sizeof(teste2);i++){
        bool capture = bit_stuffing_decoder(true,teste2[i]);         
    }
    Serial.println("");
    Serial.println("FIM DECODER");
    
    delay(10000);
}
