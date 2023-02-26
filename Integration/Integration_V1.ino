#include <AFMotor.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>     // LCD 1602 I2C용 라이브러리
// #include <Wire.h>                  // i2C 통신을 위한 라이브러리

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

    // LED set
    // pinMode(LED_R, OUTPUT);
    // pinMode(LED_L, OUTPUT);
}

// void LED_RIGHT()
// {
//     digitalWrite(LED_R, HIGH);
//     digitalWrite(LED_L, LOW);
// }

// void LED_LEFT()
// {
//     digitalWrite(LED_R, LOW);
//     digitalWrite(LED_L, HIGH);
// }

// void LED_STRAIGHT()
// {
//     digitalWrite(LED_R, HIGH);
//     digitalWrite(LED_L, HIGH);
// }

// void LED_STOP()
// {
//     digitalWrite(LED_R, LOW);
//     digitalWrite(LED_L, LOW);
// }

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
    
    if (val1 == 1 && val2 == 1) {           // 직진
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
        // LED_STRAIGHT();
    }
    else if (val1 == 1 && val2 == 0) {      // 우회전
        motor_L.run(FORWARD); 
        motor_R.run(BACKWARD);     
        // LED_RIGHT();
    }
    else if (val1 == 0 && val2 == 1) {      // 좌회전 
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
        // LED_LEFT();
    } 
    else if (val1 == 0 && val2 == 0){}      // 이전 상태 유지
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
    motor_L.setSpeed(160);
    motor_R.setSpeed(160);
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
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
        // LED_LEFT();
    }
    else if(time_current - time_previous <= 700)  // 500ms 정지
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        // LED_STOP();
    }
    else if(time_current - time_previous <= 1300) // 500ms 전진
    {
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
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
        // LED_STOP();
    }
    else if(time_current - time_previous <= 800)
    {   
        motor_L.setSpeed(180);        
        motor_R.setSpeed(180);        
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
        // LED_LEFT();
    }
    else if(time_current - time_previous > 800)
    {
        obstacle_flag = 0;
        motor_L.run(FORWARD); 
        motor_R.run(FORWARD);    
        // LED_STRAIGHT();
        time_previous = millis();            
    }
}

// 라인 미감지시 우측으로 돌기
// 장애물 감지 후 인터럽트 들어오면 obstacle_flag = 1
void IRstate5() 
{
    motor_L.setSpeed(160);
    motor_R.setSpeed(160); 
    motor_L.run(FORWARD); 
    motor_R.run(BACKWARD);
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
    }
    // OFF상태
    else if(operation_state == -1)
    {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        lcd.clear();
        lcd.noBacklight();
        // LED_STOP();
    }
}
