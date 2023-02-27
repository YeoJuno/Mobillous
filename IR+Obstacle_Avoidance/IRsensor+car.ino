#include <SoftwareSerial.h>
#include <AFMotor.h>

#define IRPin 2

// 모터드라이버 L293D  3: M3에 연결,  4: M4에 연결
AF_DCMotor motor_L(3);
AF_DCMotor motor_R(4); 

// 0: normal, 2:sensed+turning(left)
// 4:unsensed+forward(1s)+lineCheck+turning(right)
// 0->2->4->2->4->2->4->0
// line 발견시 left turning 살짝 후 4->2 
int obstacle_flag = 0; 
bool no_repetition = 0;
unsigned long Time_Previous, Time_Current;

void setup(){
    // Serial set
    Serial.begin(9600);     //시리얼 통신을 초기화합니다.

    // IRPin set
    pinMode(IRPin, INPUT);  // IRPin을 입력으로 설정합니다.
    attachInterrupt(digitalPinToInterrupt(IRPin), obstacle_detected, CHANGE);

    // Motor set
    motor_L.setSpeed(160);              // 왼쪽 모터의 속도   
    motor_L.run(RELEASE);
    motor_R.setSpeed(160);              // 오른쪽 모터의 속도   
    motor_R.run(RELEASE);    
}

// void IRsensor()
// {
//     // 가변 저항 시계방향:인식거리증가, 반시계:인식거리감소
//     // IRPin으로 읽은 조도센서의 값을 IRVal에 대입합니다.
//     int IRVal = digitalRead(IRPin);   
    
//     if (IRVal == LOW)  // LOW면 장애물 감지, 15cm정도 세팅
//     {
//         Serial.println("=== LOW ===");
//     }
//     else  // HIGH면 장애물 미감지
//     {  
//         Serial.println("ooo HIGH ooo");
//     }
// }

void car(){
    int val1 = digitalRead(A0);    // 라인센서1
    int val2 = digitalRead(A5);    // 라인센서2   
    
    if (val1 == 0 && val2 == 0) {                   // 직진
        // motor_L.setSpeed(130);
        // motor_R.setSpeed(130);
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
    }
    else if (val1 == 0 && val2 == 1) {              // 우회전 1
    // motor_L.setSpeed(130);
    motor_L.run(FORWARD); 
    motor_R.run(BACKWARD);     
    }
    else if (val1 == 1 && val2 == 0) {              // 좌회전 2
    // motor_R.setSpeed(130);
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
    } 
    else if (val1 == 1 && val2 == 1) {              // 정지
    motor_L.run(RELEASE); 
    motor_R.run(RELEASE);

    Time_Previous = millis();
    Time_Current = millis();
    }
}           


void obstacle_detected()  // IR센서 인터럽트 함수 
{ 
    // HIGH->LOW 장애물감지시작
    if(digitalRead(IRPin) == LOW)
    {
        if( no_repetition == 0)
        {
            no_repetition = 1;
            obstacle_flag = 1;
        }
    }
    // LOW -> HIGH 장애물감지벗어남  
    else if(digitalRead(IRPin) == HIGH)
    {
        no_repetition = 0;
        obstacle_flag = 3;
    }
}

void IRstate1()  // 500ms 정지
{
    motor_L.run(RELEASE);
    motor_R.run(RELEASE);
    Time_Current = millis();
    if(Time_Current - Time_Previous > 500)
    {
        Time_Previous = millis();
        obstacle_flag = 2;            
    }
}

void IRstate2()  // 회전
{
    motor_L.setSpeed(160);
    motor_R.setSpeed(160);
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
    Time_Previous = millis();
    Time_Current = millis();  
}

void IRstate3()  // 정지 -> 전진 -> 정지 -> 라인유무체크
{
    Time_Current = millis();
    if(Time_Current - Time_Previous <= 500)  // 500ms 정지
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    }
    else if(Time_Current - Time_Previous <= 1000) // 200ms 전진
    {
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
    }
    else if(Time_Current - Time_Previous <= 1500)  // 500ms 정지
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    }
    else if(Time_Current - Time_Previous > 1500)  // 라인유무 체크
    {
        int val1_check = digitalRead(A0);    // 라인센서1
        int val2_check = digitalRead(A5);    // 라인센서2
        if(val1_check == 1 || val2_check == 1)  // 1이면 감지
        {
            obstacle_flag = 4;
            Time_Current = millis();
            Time_Previous = millis();
        }
        else
        {
            obstacle_flag = 5;
        }
    }        
}

void IRstate4()  // 400ms 좌로 회전
{
    Time_Current = millis();
    if(Time_Current - Time_Previous > 300)
    {
        Time_Previous = millis();
        obstacle_flag = 0;                
    }
}

void IRstate5()  // 라인 미감지시 우측으로 돌기, 장애물 감지 후 인터럽트 들어오면 obstacle_flag = 1
{
    motor_L.setSpeed(160);
    motor_R.setSpeed(160); 
    motor_L.run(FORWARD); 
    motor_R.run(BACKWARD);
    Time_Previous = millis();
    Time_Current = millis();
}


void loop(){
    if(obstacle_flag==0){  // 일반 주행
        car();
    }
    else if(obstacle_flag==1){  // 500ms 정지 
        IRstate1();
    }
    else if(obstacle_flag == 2){  // 좌로 회전(장애물 인식 벗어나기 위해)
        IRstate2();      
    }
    else if(obstacle_flag == 3){  // 500ms 정지 후 200ms 전진
        IRstate3();
    }
    else if(obstacle_flag == 4){  // 좌로 살짝 회전
        IRstate4();
    }
    else if(obstacle_flag == 5){  // 우로 회전(장애물 인식하기 위해)
        IRstate5();
    }
}
