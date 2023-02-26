int TOUCH_PIN = 2;
// int LED_R = 5;

void setup() {
  Serial.begin(9600);
  pinMode(TOUCH_PIN, INPUT);
//   pinMode(LED_R, OUTPUT);
}

void loop() {  // 누르면 1, 떼면 0
    Serial.println(digitalRead(TOUCH_PIN));
//   if (digitalRead(TOUCH_PIN) == HIGH) {
//     // digitalWrite(LED_R, HIGH);
//   }
//   else {
//     // digitalWrite(LED_R, LOW);
//   }
    delay(200);
}
