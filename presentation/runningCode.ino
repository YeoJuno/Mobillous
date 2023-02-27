#include <mobis_io.h>
#include <AFMotor.h>

#define IRPin 2     // 장애물 감지 IR 센서

// 모터드라이버 L293D  3: M3에 연결,  4: M4에 연결
AF_DCMotor motor_L(3);
AF_DCMotor motor_R(4);

// 장애물 회피 관련 변수 선언
int obstacle_flag = 0; 
bool no_repetition = 0;
unsigned long time_previous;
unsigned long time_current;

void setup() {
    directSetPinIn(IRPin);
    attachInterrupt(digitalPinToInterrupt(IRPin), obstacle_detected, CHANGE);

    motor_L.setSpeed(180);              // 왼쪽 모터의 속도   
    motor_R.setSpeed(180);              // 오른쪽 모터의 속도   
    motor_L.run(RELEASE);
    motor_R.run(RELEASE);
    time_previous = millis();
    time_current = millis();
}
void loop() {
    if (obstacle_flag == 0) car();               // 일반 주행
    else if (obstacle_flag == 1) IRstate1();     // 500ms 정지
    else if (obstacle_flag == 2) IRstate2();     // 좌회전(장애물 인식 벗어나기 위해)
    else if (obstacle_flag == 3) IRstate3();     // 좌로 500ms 더 회전, 500ms 정지, 500ms 전진
    else if (obstacle_flag == 4) IRstate4();     // 좌로 살짝 회전
    else if (obstacle_flag == 5) IRstate5();     // 우회전(장애물 인식하기 위해)
}
// 라인트레이싱 함수
void car() {
    int val1 = directDigitalRead(A0);    // 라인센서1
    int val2 = directDigitalRead(A3);    // 라인센서2   
    if (val1 == 1 && val2 == 1) {           // 직진
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
    }
    else if (val1 == 1 && val2 == 0) {      // 우회전
        motor_L.run(FORWARD); 
        motor_R.run(BACKWARD);     
    }
    else if (val1 == 0 && val2 == 1) {      // 좌회전 
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
    } 
    else if (val1 == 0 && val2 == 0){}      // 이전 상태 유지
    time_previous = millis();
}
void obstacle_detected() { 
    if(directDigitalRead(IRPin) == LOW) {
        if( no_repetition == 0) {
            no_repetition = 1;
            obstacle_flag = 1;
        }
    } else if(directDigitalRead(IRPin) == HIGH) {
        no_repetition = 0;
        obstacle_flag = 3;
    }
}
void IRstate1() {
    motor_L.run(RELEASE);
    motor_R.run(RELEASE);
    if(time_current - time_previous > 500) {
        time_previous = millis();
        obstacle_flag = 2;            
    }
}
// 좌회전(장애물 감지 벗어날 때까지)
void IRstate2() {
    motor_L.setSpeed(160);
    motor_R.setSpeed(160);
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
    time_previous = millis(); 
}
// 좌로 좀더 회전 -> 정지 -> 전진하면서 라인유무체크 -> 정지
void IRstate3() {
    if(time_current - time_previous <= 200) { // 300ms 좌로 회전
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
    }
    else if(time_current - time_previous <= 700) { // 500ms 정지
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    }
    else if(time_current - time_previous <= 1300) { // 500ms 전진
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
        int val1_check = directDigitalRead(A0);            // 라인센서1
        int val2_check = directDigitalRead(A3);            // 라인센서2
        if (val1_check == 1 || val2_check == 1) {    // 1이면 감지
            obstacle_flag = 4;
            time_previous = millis();
        }
    }
    else if(time_current - time_previous <= 1800) { // 500ms 정지
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    }
    else if(time_current - time_previous > 1800) { // 다음 상태 진입
        obstacle_flag = 5;
    }
}

// 라인 발견 시 300ms 좌로 회전
void IRstate4() {
    if(time_current - time_previous <= 500) {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    }
    else if(time_current - time_previous <= 800) {   
        motor_L.setSpeed(180);        
        motor_R.setSpeed(180);        
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
    }
    else if(time_current - time_previous > 800) {
        obstacle_flag = 0;
        motor_L.run(FORWARD); 
        motor_R.run(FORWARD);    
        time_previous = millis();            
    }
}
// 라인 미감지시 우측으로 돌기 :: 장애물 감지 후 인터럽트 들어오면 obstacle_flag = 1
void IRstate5() {
    motor_L.setSpeed(160);
    motor_R.setSpeed(160); 
    motor_L.run(FORWARD); 
    motor_R.run(BACKWARD);
    time_previous = millis();
}
