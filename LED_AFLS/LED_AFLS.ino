int LED_R = 10;
int LED_L = 9;

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_L, OUTPUT);
}

void loop() {
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_L, HIGH);
  delay(1000);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_L, LOW);
  delay(1000);
}
