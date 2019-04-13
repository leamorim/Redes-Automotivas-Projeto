#define button1 2 //Porta digital 2 para o botão 1
#define button2 3 //Porta digital 3 para o botão 2

// variables will change:
volatile int buttonStateHardSync = 0;         // variable for reading the pushbutton status
volatile int buttonStateSoftSync = 0;         // variable for reading the pushbutton status

void setup() {
 
  Serial.begin(115200);
  pinMode(button1, INPUT);  // initialize the pushbutton 1 pin as an input:
  pinMode(button2, INPUT);  // initialize the pushbutton 2 pin as an input:

  // Attach an interrupt to the ISR vector
  attachInterrupt(0, HS_ISR, RISING);
  attachInterrupt(1, SS_ISR, RISING);
}

void loop() {
  // Nothing here!
}

void HS_ISR() {
  buttonStateHardSync = digitalRead(button1);
  Serial.println("HARD SYNC")
}

void SS_ISR() {
  buttonStateSoftSync = digitalRead(button2);
   Serial.println("SOFT SYNC")
}
