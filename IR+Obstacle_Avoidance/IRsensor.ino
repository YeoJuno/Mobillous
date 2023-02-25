int IRPin = 2;           // IRPin을 8로 선언합니다.

void setup() {
    Serial.begin(9600);     //시리얼 통신을 초기화합니다.
    pinMode(IRPin, INPUT);  // IRPin을 입력으로 설정합니다.
}

void loop() {
    int IRVal = digitalRead(IRPin);   //IRPin으로 읽은 조도센서의 값을 IRVal에 대입합니다.
    //시계방향:인식거리증가, 반시계:인식거리감소
    if (IRVal == LOW) {  // LOW 감지, 15cm정도 세팅
        Serial.println("=== LOW ===");
        }
        else {  // HIGH 미감지
        Serial.println("ooo HIGH ooo");
    }
}
