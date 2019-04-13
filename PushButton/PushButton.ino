const int button1 = 2;     // Button 1 in Digital Port 2
const int button2 = 3;     // Button 2 in Digital Port 3
int BState1 = 0;         // Button 1 State - HARD SYNC
int BState2 = 0;         // Button 2 State - SOFT SYNC

void setup() {
  Serial.begin(115200);
  // initialize the pushbutton pin as an input:
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
}

void loop() {
  // read the state of the pushbutton value:
  BState1 = digitalRead(button1);
  BState2 = digitalRead(button2);
  Serial.println("Button 1 State: ");
  Serial.println(button1);
  Serial.println("Button 2 State: ");
  Serial.println(button2);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (BState1 == HIGH) {
    // turn LED on:
    Serial.println("Button 1 State: HIGH");
  } else {
    // turn LED off:
    Serial.println("Button 1 State: LOW ");
  }
    if (BState2 == HIGH) {
    // turn LED on:
    Serial.println("Button 2 State: HIGH");
  } else {
    // turn LED off:
    Serial.println("Button 2 State: LOW ");
  }
  delay(500);
}
