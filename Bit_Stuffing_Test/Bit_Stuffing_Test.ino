enum estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE;
unsigned int count = 0;

void setup(){
    Serial.begin(115200);
    STATE = INACTIVE;
}

bool bit_stuffing_encoder(bool BS_FLAG, char BIT_TO_WRITE){

    char last_bit, CAN_TX;
    bool SEND_BIT = true;
    switch (STATE)
    {
    case INACTIVE:
        if(BS_FLAG){
            STATE = COUNTING;
            last_bit = BIT_TO_WRITE;
            CAN_TX = BIT_TO_WRITE;
            count = 1;
            SEND_BIT = true;
        }
        else{
            STATE = INACTIVE;
            CAN_TX = BIT_TO_WRITE;
            SEND_BIT = true;
        }
        break;
    
    case COUNTING:
        if(count < 5){
            if(BIT_TO_WRITE != last_bit){
                Serial.println("diferente");
                count = 1;
                CAN_TX = BIT_TO_WRITE;
                last_bit = BIT_TO_WRITE;
                SEND_BIT = true;
            }
            else{
                Serial.println("incremento");
                count++;
                //em tese as próximas duas linhas não são necessárias visto que BIT_TO_WRITE continua igual a last_bit
                CAN_TX = BIT_TO_WRITE;
                last_bit = BIT_TO_WRITE;
                SEND_BIT = true;
            }
        }
        else{ //count igual a 6
            //STATE = BIT_STUFFED;//dá pra criar um novo estado mas acho q dá pra deixar tudo nesse estado
            Serial.println("BIT STUFFED");
            SEND_BIT = false;
            count = 1;
            if(BIT_TO_WRITE == '0'){
                CAN_TX = '1';
                last_bit = '1';
            }
            else if(BIT_TO_WRITE == '1'){
                CAN_TX = '0';
                last_bit = '0';
            }
            if(BS_FLAG){
                STATE = COUNTING;
            }
            else{
                STATE = INACTIVE;
            }
        }
        break;
    }
    Serial.println("ordem: STATE/CAN_TX/count/SEND_BIT/last_bit/BIT_TO_WRITE");
    Serial.print(STATE);
    Serial.print("/");
    Serial.print(CAN_TX);
    Serial.print("/");
    Serial.print(count);
    Serial.print("/");
    Serial.print(SEND_BIT);
    Serial.print("/");
    Serial.print(last_bit);
    Serial.print("/");
    Serial.print(BIT_TO_WRITE);
    Serial.println("/");
    
    return SEND_BIT;
}

void loop(){
    char teste1 [] = {'0','0','0','0','0','0','1','1','1','1','1','1','1','0'};
    bool retorno = true;
    for(int i = 0; i < 14;){//14 eh o tamanho de teste1
        Serial.print("entrada: ");
        Serial.println(teste1[i]);
        retorno = bit_stuffing_encoder(true,teste1[i]);
        if(retorno == true){
            i++;
        }
        Serial.println(retorno);
        Serial.println("");
        Serial.println("");
    }
    Serial.println("FIM");
    Serial.println("");
    Serial.println("");
    delay(40000);
}
