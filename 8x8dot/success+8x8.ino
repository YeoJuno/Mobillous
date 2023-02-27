#include <AFMotor.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>     // LCD 1602 I2C용 라이브러리
// #include <Wire.h>                  // i2C 통신을 위한 라이브러리
#include "LedControl.h"
LedControl lc=LedControl(13,9,10,1);

#define IRPin 2
#define ONOFF 3  // 터치센서
// #define LED_R 10
// #define LED_L 9

LiquidCrystal_I2C lcd(0x27, 16, 2); // 접근주소: 0x3F or 0x27

// 모터드라이버 L293D  3: M3에 연결,  4: M4에 연결
AF_DCMotor motor_L(3);
AF_DCMotor motor_R(4); 

// 장애물 회피 관련 변수 선언
int obstacle_flag = 0; 
bool no_repetition = 0;
unsigned long time_lcdprevious, time_previous, time_current;
int turn_speed = 160;

// lcd 관련 변수 선언
bool init_flag=0;
bool onoff_flag=0;
int operation_state = 1;
bool mode = 1; 
int distance = 100;
int velocity = 0;
int digit_distance = 12;
int digit_velocity = 9;
bool mode_tmp = 1;
int digit_distance_tmp = 0;
int digit_velocity_tmp = 0;
int setcursor_tmp = 0;
String string_mode;
String string_distance;
String string_velocity;

// 현재 모터 속도
int now_motorL_speed = 180;
int now_motorR_speed = 180;

// 8x8 dotmatrix
byte light_off = {B00000000};
byte light_on = {B11111111};
int turn_intensity = 0;  
int turn_intensity_tmp = 0;  
unsigned long light_time_previous, light_time_current;
int light_count=0;
bool light_onoff_flag = true;
int turn_state=0;  // -1 좌, 0 직진+정지, 1 우
int turn_state_tmp=0;  // 100ms 이전 turn_state 저장
int test_val=0;
int test_flag=1;

// 셋업
void setup(){
    // Serial set
    // Serial.begin(9600);     //시리얼 통신을 초기화합니다.

    // IRPin set
    pinMode(IRPin, INPUT);  // IRPin을 입력으로 설정합니다.
    attachInterrupt(digitalPinToInterrupt(IRPin), obstacle_detected, CHANGE);

    // Motor set
    motor_L.setSpeed(180);              // 왼쪽 모터의 속도   
    motor_L.run(RELEASE);
    motor_R.setSpeed(180);              // 오른쪽 모터의 속도   
    motor_R.run(RELEASE);
    time_previous = millis();
    time_current = millis();
    time_lcdprevious = millis();

    // LCD set
    lcd.init();  // LCD 초기화
    lcd.backlight();
    pinMode(ONOFF, INPUT);
    digitalWrite(ONOFF, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ONOFF), onoff, CHANGE);

    // 8x8 dot
    lc.shutdown(0,false);
    lc.setIntensity(0,2);
    lc.clearDisplay(0);
    lc.setRow(0, 3, light_on);
    lc.setRow(0, 4, light_on);
    
}


/*
light_onoff_flag값에 따라서 on/off 결정
light_onoff_flag == true :ON
light_onoff_flag == false : OFF

turn_intensity 값에 따라 좌,우 결정
좌회전 lc.setRow(0, 7, light_on); 3단계 turn_intensity == -4
좌회전 lc.setRow(0, 6, light_on); 2단계 turn_intensity == -3
좌회전 lc.setRow(0, 5, light_on); 1단계 turn_intensity == -2
우회전 lc.setRow(0, 2, light_on); 1단계 turn_intensity == 1
우회전 lc.setRow(0, 1, light_on); 2단계 turn_intensity == 2
우회전 lc.setRow(0, 0, light_on); 3단계 turn_intensity == 3
*/
void light_state(){
    // if(turn_intensity==0 || turn_intensity == -1) return;
    
    if(turn_intensity <= -1){
        if(turn_intensity - turn_intensity_tmp < 0){ // 더 좌회전
            lc.setRow(0, 3-turn_intensity, light_on);
        }
        else if(turn_intensity - turn_intensity_tmp > 0){ // 덜 좌회전
            lc.setRow(0, 3-turn_intensity_tmp, light_off);
        }
    }
    else if(turn_intensity >= 0){  // 우회전 중일 때
        if(turn_intensity - turn_intensity_tmp > 0){ // 더 우회전
            lc.setRow(0, 3-turn_intensity, light_on);
        }
        else if(turn_intensity - turn_intensity_tmp < 0){ // 덜 우회전
            lc.setRow(0, 3-turn_intensity_tmp, light_off);            
        }
    }
    
}

// void TEST(){
//     if(test_val > 20){
//         turn_state = -1;
//         test_flag = -1;
//     }
//     else if(test_val < -20){
//         turn_state = 1;
//         test_flag = 1;
//     }
// }

void dotmatrix(){
    light_time_current = millis();

    if(light_time_current - light_time_previous >= 50){
        light_time_previous = light_time_current;

        // TEST();
        // test_val += test_flag;        

        // 현재 좌우 정보 확인
        if(turn_state == 1){
            light_count+=1;
            turn_state_tmp = turn_state;
        }                
        else if(turn_state == -1){
            light_count-=1;
            turn_state_tmp = turn_state;
        }
        else if(turn_state == 0){  // 직진이면 직진 신호로 상쇄
            light_count -= turn_state_tmp;
        }

        /* 
        다섯 번 우회전 신호가 누적되면 turn_intensity 증가(우회전)
        다섯 번 좌회전 신호가 누적되면 turn_intensity 감소(좌회전)
        light_count 초기화
        */
        if(light_count >=5){
            turn_intensity_tmp = turn_intensity;
            turn_intensity+=1;
            if(turn_intensity > 3) turn_intensity = 3; // limit
            light_count=0;
            light_state();
        }
        else if(light_count <=-5){
            turn_intensity_tmp = turn_intensity;
            turn_intensity-=1;
            if(turn_intensity < -4) turn_intensity = -4; // limit
            light_count=0;
            light_state();
        }
    }
}

// 자릿수 체크
int digit_check(int num)     
{
    if(num == 0) return 1;
    
    int cnt = 0;
    while(num!=0){
        num/=10;
        cnt++;
    }
    return cnt;
}

// 주행거리 입력
void writeLCD_distance()
{
    setcursor_tmp = 13 - digit_distance;
    lcd.setCursor(setcursor_tmp,0);
    lcd.print(string_distance);
    lcd.setCursor(14,0);
    lcd.print("cm");        
}

// 속도 입력
void writeLCD_velocity()
{
    setcursor_tmp = 10 - digit_velocity;
    lcd.setCursor(setcursor_tmp, 1);
    lcd.print(string_velocity);
    lcd.setCursor(11,1);
    lcd.print("cm/s");        
}

// 모드 입력
void writeLCD_mode()       
{
    lcd.setCursor(1,0);
    if(mode){
        lcd.print("SPEED");
    }
    else {
        lcd.print("SAFETY");
    }
}

// LCD 클러스터
void cluster()    
{
    lcd.backlight();

    if(init_flag==1) lcd.clear();
    if(distance < 0) distance = 0;
    if(velocity < 0) velocity = 0;

    digit_distance = digit_check(distance);  // 현재 distance 자릿 수를 저장
    digit_velocity = digit_check(velocity);  // 현재 velocity 자릿 수를 저장
    
    if(digit_distance < digit_distance_tmp) lcd.clear();
    if(digit_velocity < digit_velocity_tmp) lcd.clear();
    // if(mode != mode_tmp) lcd.clear();
    
    string_distance = String(distance);
    string_velocity = String(velocity);
    string_mode = String(mode);

    lcd.clear();
    writeLCD_mode();
    writeLCD_distance();
    writeLCD_velocity();

    // test용
    distance += 30;
    velocity += 3;

    digit_distance_tmp = digit_distance;
    digit_velocity_tmp = digit_velocity;
    // mode_tmp = mode;
       
}

// 동작 on/off 인터럽트 함수
void onoff()
{
    if(digitalRead(ONOFF) == HIGH)
    {
        if(onoff_flag==0)
        {
            onoff_flag = 1;      
            operation_state = -operation_state;
            distance = 0;
            velocity = 0;
            // Serial.println(onoff_flag);
        }
        else{}
    }
    else
    {
        onoff_flag = 0;               
        // Serial.println(onoff_flag);
    }
}

// 라인트레이싱 함수
void car()
{
    int val1 = digitalRead(A0);    // 라인센서1
    int val2 = digitalRead(A3);    // 라인센서2   
    
    // if (val1 == 1 && val2 == 1) {           // 직진
    //     motor_L.run(FORWARD);
    //     motor_R.run(FORWARD);
    //     turn_state = 0;
    //     // LED_STRAIGHT();
    // }
    // else if (val1 == 0 && val2 == 1) {      // 우회전
    //     motor_L.run(FORWARD); 
    //     motor_R.run(BACKWARD);   
    //     turn_state = 1;  
    //     // LED_RIGHT();
    // }
    // else if (val1 == 1 && val2 == 0) {      // 좌회전 
    //     motor_L.run(BACKWARD); 
    //     motor_R.run(FORWARD);
    //     turn_state = -1;
    //     // LED_LEFT();
    // } 
    // else if (val1 == 0 && val2 == 0){}      // 이전 상태 유지

    if (val1 == 1 && val2 == 1) {                   // 직진
       motor_L.setSpeed(255);
       motor_R.setSpeed(255);
       motor_L.run(FORWARD); 
       motor_R.run(FORWARD);
    //    Serial.println("F");
      }
      else if (val1 == 0 && val2 == 1) {              // 우회전 1
       motor_R.setSpeed(105); 
       motor_L.run(FORWARD); 
       motor_R.run(BACKWARD);
    //    Serial.println("->");       
      }
      else if (val1 == 1 && val2 == 0) {              // 좌회전 2
        motor_L.setSpeed(105);
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
        // Serial.println("<-");
      } 
      else if (val1 == 0 && val2 == 0) {              // 정지
        // motor_R.setSpeed(160);
        // motor_L.setSpeed(160);
      }

    time_previous = millis();
}           

// IR센서 인터럽트 함수 
void obstacle_detected()  
{ 
    // HIGH->LOW 장애물감지함
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

// 500ms 정지
void IRstate1()  
{
    motor_L.run(RELEASE);
    motor_R.run(RELEASE);
    turn_state = 0;
    // LED_STOP();
    if(time_current - time_previous > 500)
    {
        time_previous = millis();
        obstacle_flag = 2;            
    }
}

// 좌로회전(장애물 감지 벗어날 때까지)
void IRstate2()  
{
    motor_L.setSpeed(turn_speed);
    motor_R.setSpeed(turn_speed);
    now_motorL_speed = turn_speed;
    now_motorR_speed = turn_speed;
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
    turn_state = -1;
    
    // LED_LEFT();
    time_previous = millis(); 
}

// 좌로 좀더 회전 -> 정지 -> 전진하면서 라인유무체크 -> 정지
void IRstate3()  
{
    if(time_current - time_previous <= 200)  // 300ms 좌로 회전
    {
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
        turn_state = -1;
        // LED_LEFT();
    }
    else if(time_current - time_previous <= 700)  // 500ms 정지
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        turn_state = 0;
        // LED_STOP();
    }
    else if(time_current - time_previous <= 1300) // 500ms 전진
    {
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
        turn_state = 0;
        // LED_STRAIGHT();
        int val1_check = digitalRead(A0);    // 라인센서1
        int val2_check = digitalRead(A3);    // 라인센서2
        if(val1_check == 1 || val2_check == 1)  // 1이면 감지
        {
            obstacle_flag = 4;
            time_previous = millis();
        }
    }
    else if(time_current - time_previous <= 1800)  // 500ms 정지
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        turn_state = 0;
        // LED_STOP();
    }
    else if(time_current - time_previous > 1800)  // 다음 상태 진입
    {
        obstacle_flag = 5;
    }        
}

// 라인 발견 시 300ms 좌로 회전
void IRstate4()  
{
    if(time_current - time_previous <= 500)
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        turn_state = 0;
        // LED_STOP();
    }
    else if(time_current - time_previous <= 800)
    {   
        motor_L.setSpeed(180);        
        motor_R.setSpeed(180);
        now_motorL_speed = 180;        
        now_motorR_speed = 180;        
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
        turn_state = -1;
        // LED_LEFT();
    }
    else if(time_current - time_previous > 800)
    {
        obstacle_flag = 0;
        motor_L.setSpeed(255);        
        motor_R.setSpeed(255);        
        motor_L.run(FORWARD); 
        motor_R.run(FORWARD);   
        turn_state = 0; 
        // LED_STRAIGHT();
        time_previous = millis();            
    }
}

// 라인 미감지시 우측으로 돌기
// 장애물 감지 후 인터럽트 들어오면 obstacle_flag = 1
void IRstate5() 
{
    motor_L.setSpeed(turn_speed);
    motor_R.setSpeed(turn_speed); 
    now_motorL_speed = turn_speed;
    now_motorR_speed = turn_speed;
    motor_L.run(FORWARD); 
    motor_R.run(BACKWARD);
    turn_state = 1;
    // LED_RIGHT();
    time_previous = millis();
}

// 메인 루프
void loop()
{
    // Serial.println(obstacle_flag);
    
    // ON 상태
    if(operation_state == 1)
    {
        time_current = millis();
        
        // LCD cluster
        if(time_current - time_lcdprevious >= 1000){  // 1초에 한 번씩 동작
            time_lcdprevious = time_current;
            cluster();                             
        }        

        // 장애물 회피
        if(obstacle_flag==0){  // 일반 주행
            car();
        }
        else if(obstacle_flag==1){  // 500ms 정지 
            IRstate1();
        }
        else if(obstacle_flag == 2){  // 좌로 회전(장애물 인식 벗어나기 위해)
            IRstate2();      
        }
        else if(obstacle_flag == 3){  // 좌로 500ms 더 회전, 500ms 정지, 500ms 전진
            IRstate3();
        }
        else if(obstacle_flag == 4){  // 좌로 살짝 회전
            IRstate4();
        }
        else if(obstacle_flag == 5){  // 우로 회전(장애물 인식하기 위해)
            IRstate5();
        }

        // 8x8 dotmatrix
        dotmatrix();
    }
    // OFF상태
    else if(operation_state == -1)
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        turn_state = 0;
        lcd.clear();
        lcd.noBacklight();
        // LED_STOP();
    }
}