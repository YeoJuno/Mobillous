#include <mobis_io.h>
#include <mobis_motor.h>
#include <LiquidCrystal_I2C.h>

/* ************************************************ */
/* [모듈 1] 라인트레이싱 및 장애물 회피 관련 선언부 */
#define IRPin 2                 // 장애물 감지 IR 센서 핀번호
bool no_repetition      = 0;    // 
uint8_t obstacle_flag   = 0;    // 
unsigned long time_current;     // 
unsigned long time_previous;    // 
unsigned long time_lcdprevious; // 

void car( void );
void IRstate1 ( void );
void IRstate2 ( void );
void IRstate3 ( void );
void IRstate4 ( void );
void IRstate5 ( void );
void obstacle_detected( void );
/* ************************************************ */

/* ************************************************ */
/* [모듈 2] LCD 관련 선언부                         */
#define ONOFF 3                         // 터치 센서 핀번호
LiquidCrystal_I2C lcd(0x27, 16, 2);     // I2C 주소: (또는 0x3F)

bool mode                   = true;
bool mode_tmp               = true;
bool init_flag              = false;
bool onoff_flag             = false;
bool operation_state        = true;
uint8_t distance            = 100;
uint8_t digit_distance      = 12;
uint8_t digit_distance_tmp  = 0;
uint8_t velocity            = 0;
uint8_t digit_velocity      = 9;
uint8_t digit_velocity_tmp  = 0;
uint8_t setcursor_tmp       = 0;
String string_mode;
String string_distance;
String string_velocity;

int digit_check( int num );
void writeLCD_mode( void );
void writeLCD_distance( void );
void writeLCD_velocity( void );
void cluster( void );
void onoff( void );         // ISR handler function
/* *********************************************** */

/* ************************************************ */
/* [모듈 3] 모터 관련 선언부                        */
#define IRPin 2         // 장애물 감지 IR 센서 핀번호
Motor motor_L(3);       // L293D Channel 3에 연결
Motor motor_R(4);       // L293D Channel 4에 연결
uint8_t left_IR_value   = 0;    // 좌측 Linetrace IR 센서값
uint8_t right_IR_value  = 0;    // 우측 Linetrace IR 센서값
uint8_t left_motor_speed = 0;
uint8_t right_motor_speed = 0;
/* ************************************************ */

/* ************************************************ */
/* [모듈 4] PID 관련 선언부                         */
#define num_samples 50             // Number of samples to use for filtering
int sensor_index = 0;               // Index for storing sensor data in array
int sensor_data[num_samples];       // Array to store sensor data for filtering

uint8_t max_speed       = 255;
uint8_t min_speed       = 120;
uint8_t diff_speed      = 20;
int kp                  = 20;
int ki                  = 4;
int kd                  = 2;
int error               = 0;
int last_error          = 0;
int total_error         = 0;
float total             = 0;
float filter_boundary   = 0.3;

float filterSensorData(int sensor_value);
/* ************************************************ */

/* ***** Arduino setup() & loop() 시작 ***************** */
void setup() {
    /* 라인트레이서를 위한 IR 센서 설정 - Pin A0와 A3 포트 방향을 INPUT으로 */

    Serial.begin(9600);
    directSetPinIn(A0);
    directSetPinIn(A3);
    time_previous = millis();
    time_current = time_previous;
    time_lcdprevious = time_previous;
    /* 장애물 인지를 위한 IR 센서 설정
    IRPin(Pin 2) 포트 방향을 INPUT으로, 인터럽트 정의 */
    directSetPinIn(IRPin);
    attachInterrupt(digitalPinToInterrupt(IRPin), obstacle_detected, CHANGE);
    /* 모터 구동을 위한 모터 초기화  */
    motor_L.setSpeed(max_speed);  motor_L.run(RELEASE);   // 좌측 모터 초기화
    motor_R.setSpeed(max_speed);  motor_R.run(RELEASE);   // 오른쪽 모터 초기화
    /* LCD 구동을 위한 패널 초기화  */
    lcd.init();
    lcd.backlight();
    /* LCD 구동을 위한 터치센서 초기화
    ONOFF(Pin 3) 포트 방향을 INPUT으로, 인터럽트 정의  */
    directSetPinIn(ONOFF);
    attachInterrupt(digitalPinToInterrupt(ONOFF), onoff, CHANGE);
}
void loop() {
    /* 터치센서 ON - 자동차 엔진 시동이 걸린 상태 */
    if (operation_state) {
        /* LCD 클러스터를 1초에 한 번씩 갱신 */
        time_current = millis();
        if (time_current - time_lcdprevious >= 1000) {
            time_lcdprevious = time_current;
            cluster();
        }
        /* 주행 모드를 결정 후 주행 */
        switch(obstacle_flag) {
        case 0: car(); break;       // 일반 주행
        case 1: IRstate1(); break;  // 500ms 정지
        case 2: IRstate2(); break;  // 장애물 인식 벗어나기 위한 좌회전 (방향 전환)
        case 3: IRstate3(); break;  // 500ms 좌회전, 500ms 정지, 500ms 주행
        case 4: IRstate4(); break;  // 좌회전
        case 5: IRstate5(); break;  // 장애물 인식 위한 우회전(방향 전환)
        }
    }
    /* 터치센서 OFF - 자동차 엔진 시동이 꺼진 상태 */ 
    else {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        lcd.clear();
        lcd.noBacklight();
    }
}

/* ***** Arduino setup() & loop() 끝 ***************** */
/* *************************************************** */

/* *************************************************** */
/* ***** [모듈 1] 라인트레이싱 관련 함수 정의부 시작** */

void car() {
    left_IR_value = directDigitalRead(A0);
    right_IR_value = directDigitalRead(A3);

    int diff_IR_value = left_IR_value - right_IR_value;
    
    float filtered_error = filterSensorData(diff_IR_value);
    total_error += filtered_error;
    last_error = filtered_error;
    
    /*int pid_value = (kp * filtered_error) + (ki * total_error) + (kd * (filtered_error - last_error));
    
    int left_speed = max_speed - (pid_value * 2);
    if (left_speed > max_speed) left_speed = max_speed;
    if (left_speed < min_speed) left_speed = min_speed;
    
    int right_speed = max_speed + 2 * pid_value;
    if (right_speed > max_speed) right_speed = max_speed;
    if (right_speed < min_speed) right_speed = min_speed;
    Serial.println(filtered_error);*/
    // 라인센서2   

      if ((left_IR_value || right_IR_value) &&((-1) * filter_boundary <= filtered_error && filtered_error <= filter_boundary)) {  
        motor_L.setSpeed(max_speed);
        motor_R.setSpeed(max_speed);                // 직진
        motor_L.run(FORWARD); 
        motor_R.run(FORWARD);
      }
      else if (left_IR_value == 1 && right_IR_value == 0) {  
        motor_R.setSpeed(max_speed - filtered_error *20) ;         
        motor_R.run(FORWARD); 
        motor_L.setSpeed(min_speed - filtered_error * 22);
        if (min_speed - filtered_error * 20 < 108){
            motor_L.run(BACKWARD);
            motor_L.setSpeed(min_speed - 30 + filtered_error * 15);
            
        }
        else motor_L.run(FORWARD); // BACK
      }
      else if (left_IR_value == 0 && right_IR_value == 1) {   
        motor_L.setSpeed(max_speed + filtered_error *20)  ;        
        motor_L.run(FORWARD); 
        motor_R.setSpeed(min_speed + filtered_error * 22);
        if (min_speed + filtered_error * 20 < 108){
            motor_R.run(BACKWARD);
            motor_R.setSpeed(min_speed - 30 - filtered_error * 15);
            
        }
        else motor_R.run(FORWARD); // BACK
      } 
    time_previous = millis();
}
// IR센서 인터럽트 함수 
void obstacle_detected() { 
    // HIGH->LOW 장애물감지함
    if (directDigitalRead(IRPin) == LOW) {
        if (no_repetition == false) {
            no_repetition = true;
            obstacle_flag = 1;
        }
    }
    // LOW -> HIGH 장애물감지벗어남
    else if (directDigitalRead(IRPin) == HIGH) {
        no_repetition = false;
        obstacle_flag = 3;
    }
}
// 500ms 정지
void IRstate1 () {
    motor_L.run(RELEASE);
    motor_R.run(RELEASE);
    time_current = millis();
    if(time_current - time_previous > 500) {
        time_previous = time_current;
        obstacle_flag = 2;            
    }
}
// 좌회전 (장애물 감지 벗어날 때까지)
void IRstate2 () {
    motor_L.setSpeed(160);
    motor_R.setSpeed(160);
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
    time_previous = millis();
    time_current = time_previous;
}
// 좌로 좀더 회전 -> 정지 -> 전진하면서 라인유무체크 -> 정지
void IRstate3 () {
    time_current = millis();
    unsigned long time_diff = time_current - time_previous;
    if (time_diff <= 200) {           // 200ms 좌회전
        motor_L.run(BACKWARD);
        motor_R.run(FORWARD);
    } else if (time_diff <= 700) {    // 500ms 정지
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    } else if (time_diff <= 1300)  {  // 500ms 전진
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
        left_IR_value = directDigitalRead(A0);
        right_IR_value = directDigitalRead(A3);
        if (left_IR_value || right_IR_value) {
            obstacle_flag = 4;
            time_previous = millis();
        }
    } else if(time_diff <= 1800) {     // 500ms 정지
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    } else {      // 다음 상태 진입
        obstacle_flag = 5;
    }
}
// 라인 발견 시 300ms 좌로 회전
void IRstate4 () {
    time_current = millis();
    unsigned long time_diff = time_current - time_previous;
    if(time_diff <= 500) {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    } else if(time_diff <= 800) {   
        motor_L.setSpeed(180);    
        motor_R.setSpeed(180);
        motor_L.run(BACKWARD);
        motor_R.run(FORWARD);
    } else {
        obstacle_flag = 0;
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
        time_previous = millis();
    }
}
// 라인 미감지시 우측으로 돌기
// 장애물 감지 후 인터럽트 들어오면 obstacle_flag = 1
void IRstate5() {
    motor_L.setSpeed(160);
    motor_R.setSpeed(160);
    motor_L.run(FORWARD);
    motor_R.run(BACKWARD);
    time_previous = millis();
    time_current = time_previous;
}

/* ***** [모듈 1] 라인트레이싱 관련 함수 정의부 종료** */
/* *************************************************** */

/* *************************************************** */
/* ***** [모듈 2] LCD 관련 함수 정의부 시작*********** */

/* 16-bit 정수의 자릿수를 반환 */
int digit_check(int num) {
    if(num == 0) return 1;
    int cnt = 0;
    while( num != 0 ) { num /= 10; cnt++; }
    return cnt;
}
/* LCD에 주행거리 정보 출력 */
void writeLCD_distance() {
    setcursor_tmp = 13 - digit_distance;
    lcd.setCursor(setcursor_tmp,0);
    lcd.print(string_distance);
    lcd.setCursor(14,0);
    lcd.print("cm");        
}
/* LCD에 속도 정보 출력 */
void writeLCD_velocity() {
    setcursor_tmp = 10 - digit_velocity;
    lcd.setCursor(setcursor_tmp, 1);
    lcd.print(string_velocity);
    lcd.setCursor(11,1);
    lcd.print("cm/s");
}
/* LCD에 모드 정보 출력 */
void writeLCD_mode() {
    lcd.setCursor(1, 0);
    if (mode) lcd.print("SPEED");
    else lcd.print("SAFETY");
}
// LCD 클러스터
void cluster() {
    lcd.backlight();

    if (init_flag==1) lcd.clear();

    digit_distance = digit_check(distance);  // 현재 distance 자릿수를 저장
    digit_velocity = digit_check(velocity);  // 현재 velocity 자릿수를 저장
    
    if (digit_distance < digit_distance_tmp) lcd.clear();
    if (digit_velocity < digit_velocity_tmp) lcd.clear();


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
}

// 동작 on/off 인터럽트 함수
void onoff() {
    if (directDigitalRead(ONOFF)) {
        if (!onoff_flag) {
            onoff_flag = true;
            distance = 0;
            velocity = 0;
            operation_state = !operation_state;
        } else { }
    } else {
        onoff_flag = false;
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
    }
}

/* ***** [모듈 2] LCD 관련 함수 정의부 종료*********** */
/* *************************************************** */

/* *************************************************** */
/* ***** [모듈 4] PID 관련 함수 정의부 시작*********** */

/* Function to filter sensor data using a rolling average */
float filterSensorData(int sensor_value) {
    total -= sensor_data[sensor_index];
    total += sensor_value;
    sensor_data[sensor_index] = sensor_value;
    sensor_index = (sensor_index + 1) % num_samples;
    return total / num_samples;
}

/* *************************************************** */
/* ***** [모듈 4] PID 관련 함수 정의부 종료*********** */