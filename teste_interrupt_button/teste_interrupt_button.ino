void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  attachInterrupt(0,buttone,RISING);
  attachInterrupt(1,butttwo,RISING);
}

void buttone(){
  Serial.println("Interrupt Button ONE");
}

void butttwo(){
  Serial.println("Interrupt Button TWO");
}
void loop() {
  while(true){
      Serial.print(".");
      delay(250);
  }  
}
