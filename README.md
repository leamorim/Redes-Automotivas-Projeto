## Repositório do projeto Redes Automotivas 2019.1 - Protocolo CAN

### Alunos: Belarmino Lira (bgl), Douglas Tavares (dtrps), Lucas Amorim (lea)

Bit_Timing_Module_Code_2 o TimerOne chama a função Inc_Count, configurada em  Timer1.attachInterrupt(Inc_Count), que incrementa o contador após o tempo de TQ setado em Timer1.initialize(TQ). 
Inc_Count tem um sinal global reset_count que é usado para zerar o valor do contador através da UC.
A função Bit_Timing_Module() chama o Edge_Detector, que procura um hard ou soft sync, pega os resultados dela e passa para UC. Aqui deveria ter também o TQ_Configurator_Module. Bit_Timing_Module() é chamado dentro do loop, enquanto o Inc_Count só é chamado pelo TimerOne após o tempo de 1 TQ passar. 

Bit_Timing_Module_Code_1 o TimerOne chama a UC, onde o contador e a lógica de troca de estados está. Neste caso foi necessário ter várias outras variáveis globais para poder funcionar, uma vez que a UC não está dentr do Loop.

## Falta: 
1. Verificar Soft_Sync em ambas implementações(Hard_Sync funciona nas duas já)
2. Adicionar TQ_Configurator no Code_2
3. Adicionar código para o botão
4. Arrumar um jeito de plotar no serial plotter do arduíno.

## O que deve ser mostrado no plotter:

1. writing point
2. sample point
3. tq
4. estado atual
5. hard_sync
6. soft_sync





