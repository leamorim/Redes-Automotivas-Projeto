    #include <TimerOne.h>
    #include <SoftwareSerial.h>


    char CAN_TX = '\0';
    char CAN_RX = '\0';
    bool BUS_IDLE_FLAG = true;

  enum end_dec_estados {BUS_IDLE = 0,SoF = 1,ID_A = 2,RTR_SRR = 3,IDE_0 = 4,R0 = 5, DLC = 6,
    DATA = 7, CRC_READ = 8,CRC_DELIMITER = 9, ACK_SLOT = 10, ACK_DELIMITER, EoF,
    INTERFRAME_SPACING,IDE_1, ID_B,RTR, R1R0, STATE_ERROR,
    FORMAT_ERROR, ACK_ERROR, CRC_ERROR, BIT_STUFFING_ERROR, STATE_BSD_FLAG1,
    BIT_ERROR , STATE_BED_FLAG1, OVERLOAD, WAIT , SOF, IDE, crce, SRR,R1,R2,
    IDB,ERROR_FLAG_STATE, ERROR_DELIMITER,
    OVERLOAD_DELIMITER, OVERLOAD_FLAG_STATE, ARBITRATION_LOSS_STATE} STATE_DEC, STATE_ENC;

  enum bs_estados {INACTIVE = 0,COUNTING,BIT_STUFFED} STATE_BS_ENC,STATE_BS_DEC;

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


//Decoder BEGIN

    #define L_BIT 1
    #define L_ID_A 11
    #define L_ID_B 18
    #define L_R1R0 2
    #define L_DLC 4
    #define L_DATA 8*Value_DLC
    #define L_CRC 15
    #define L_EOF 7
    #define L_INTERFRAME_SPACING 3

    // Bit Stuffing Decoder BEGIN

    unsigned int count_bs_encoder = 0;
    unsigned int count_bs_decoder = 0;
    char last_bit_dec;

    char BIT_TO_SAVE = '\0';
    bool CAPTURE,BSE_FLAG, BSD_FLAG = true; 
    // Bit Stuffing Decoder END


    //Decoder Variáveis BEGIN
    unsigned int count_decoder = 0;
    bool ERROR_FLAG = false;
    bool BED_FLAG = false;
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

  
    //Decoder Variáveis END
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


//Decoder END



//Bit_Timing_Module BEGIN
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

    char *crc;

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



    void func_sample_point(){
        bit_stuffing_decoder(CAN_RX);
        //Serial.print("BIT_TO_SAVE: ");
        //Serial.println(BIT_TO_SAVE);
        // Serial.print("capture: ");
        //Serial.println(CAPTURE);
        UC_DECODER();
     //   check_arbitration();//funçao q checa arbitração
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
        //print_state(); // função para debugar q n foi adicionada ao all
        Writing_Point = false;
        Sample_Point = false;

        switch(STATE_BT){
            case SYNC:
            //Serial.println("SYNC");
            Writing_Point = true;
           // func_writing_point();
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
                Serial.print("CAN_RX = ");
                Serial.println(CAN_RX);
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
    Serial.begin(115200);
    Timer1.initialize(TQ);
    Timer1.attachInterrupt(UC_BT);
    STATE_BT = SYNC;//State Bit Timing UC
    STATE_DEC = BUS_IDLE;//STATE DO DECODER
    STATE_BS_DEC = INACTIVE;//State do Bit Stuffing do Decoder
        //  STATE_BS_ENC = INACTIVE;//State do BS do Encoder
        //  STATE_SEND = FORMAT_SEND;
    count_bt = 0;//contador do Bit Timing

    //Comunicação Serial
     //Comunicação Serial
    pinMode(CAN_RX_PIN,INPUT);
    pinMode(CAN_TX_PIN,OUTPUT);
    mySerial.begin(115200);
    
    Serial.println("Setup OK" );
    }
//Setup END

void loop(){

}
