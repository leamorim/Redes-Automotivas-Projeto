//Bit Stuffing module

enum STATE {INACTIVE = 0,COUNTING,BIT_STUFFED}

bool bit_stuffing_encoder(bool BS_FLAG, bool BIT_TO_WRITE){

    bool last_bit, CAN_TX;
    unsigned int count = 0;

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
        if(count <= 5){
            if(BIT_TO_WRITE != last_bit){
                count = 1;
                CAN_TX = BIT_TO_WRITE;
                last_bit = BIT_TO_WRITE;
                SEND_BIT = true;
            }
            else{
                count++;
                //em tese as próximas duas linhas não são necessárias visto que BIT_TO_WRITE continua igual a last_bit
                CAN_TX = BIT_TO_WRITE;
                last_bit = BIT_TO_WRITE;
                SEND_BIT = true;
            }
        }
        else{ //count igual a 6
            //STATE = BIT_STUFFED;//dá pra criar um novo estado mas acho q dá pra deixar tudo nesse estado
            SEND_BIT = false;
            CAN_TX = !BIT_TO_WRITE;
            last_bit = !BIT_TO_WRITE;
            count = 1;

            if(BS_FLAG){
                STATE = COUNTING;
            }
            else{
                STATE = INACTIVE;
            }
        }
        break;

    printf("STATE: %d\n",STATE);
    printf("CAN_TX: %d\n",CAN_TX);
    printf("count == %d\n",count);
    return SEND_BIT;
}

/*
void bit_stuffing_decoder(){

}
*/

void main(){
    char teste1 [] = {'0','0','0','0','0','0','1','1','1','1','1','1'};

    for(int i = 0; i < 11;i++){
        printf("entrada: %d",teste1[i]);
        bool retorno = bit_stuffing_encoder(true,teste1[i]);
        printf(x ? "true\n" : "false\n");
    }
    printf("fim\n");
}