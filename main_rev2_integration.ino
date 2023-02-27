#include <mobis_io.h>
#include <mobis_motor.h>
#include <LiquidCrystal_I2C.h>      // LCD 1602 I2C용 라이브러리

#define IRPin 2
#define ONOFF 3  // 터치센서
// #define LED_R 10
// #define LED_L 9

LiquidCrystal_I2C lcd(0x27, 16, 2); // 접근주소: 0x3F or 0x27

// 모터드라이버 L293D  3: M3에 연결,  4: M4에 연결
Motor motor_L(3);
Motor motor_R(4);

// 장애물 회피 관련 변수 선언
bool no_repetition      = 0;
uint8_t obstacle_flag       = 0; 
unsigned long time_lcdprevious, time_previous, time_current;

// lcd 관련 변수 선언
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

// 라인트레이싱 관련 변수 선언
uint8_t left_IR_value   = 0;
uint8_t right_IR_value  = 0;

// PID 관련 변수 선언
const int num_samples = 100;        // Number of samples to use for filtering
int sensor_data[num_samples];       // Array to store sensor data for filtering
int sensor_index = 0;               // Index for storing sensor data in array

uint8_t max_speed   = 250;
uint8_t min_speed   = 160;
uint8_t speed_diff  = 0;
int kp              = 204;
int ki              = 255;
int kd              = 2000;
int error           = 0;
int last_error      = 0;
int total_error     = 0;
float total         = 0;
float filter_boundary = 0.3;


// 자릿수 체크
int digit_check(int num) {
    if(num == 0) return 1;

    int cnt = 0;
    while( num != 0 ) {
        num /= 10;
        cnt++;
    }
    return cnt;
}
// 주행거리 입력
void writeLCD_distance() {
    setcursor_tmp = 13 - digit_distance;
    lcd.setCursor(setcursor_tmp,0);
    lcd.print(string_distance);
    lcd.setCursor(14,0);
    lcd.print("cm");        
}
// 속도 입력
void writeLCD_velocity() {
    setcursor_tmp = 10 - digit_velocity;
    lcd.setCursor(setcursor_tmp, 1);
    lcd.print(string_velocity);
    lcd.setCursor(11,1);
    lcd.print("cm/s");        
}
// 모드 입력
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
    
    if(digit_distance < digit_distance_tmp) lcd.clear();
    if(digit_velocity < digit_velocity_tmp) lcd.clear();
    // if (mode != mode_tmp) lcd.clear();
    
    // string_distance = (char *)(distance);
    // string_velocity = (char *)(velocity);
    // string_mode = (char *)(mode);

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
    if(digitalRead(ONOFF) == HIGH && !onoff_flag) {
        onoff_flag = 1;
        distance = 0;
        velocity = 0;
        operation_state = -operation_state;
    } else {
        onoff_flag = 0;
    }
}
// Function to filter sensor data using a rolling average
float filterSensorData(int sensor_value) {
    total -= sensor_data[sensor_index];
    total += sensor_value;
    sensor_data[sensor_index] = sensor_value;
    sensor_index = (sensor_index + 1) % num_samples;
    return total / num_samples;
}
// 라인트레이싱 함수
void car() {
    left_IR_value = directDigitalRead(A0);
    right_IR_value = directDigitalRead(A3);

    int diff_IR_value = left_IR_value - right_IR_value;
    
    float filtered_error = filterSensorData(diff_IR_value);
    total_error += filtered_error;
    last_error = filtered_error;
    
    int pid_value = (kp * filtered_error) + (ki * total_error) + (kd * (filtered_error - last_error));
    
    int left_speed = max_speed - (pid_value * 3);
    if (left_speed > max_speed) left_speed = max_speed;
    if (left_speed < min_speed) left_speed = min_speed;
    
    int right_speed = max_speed + 4 * pid_value;
    if (right_speed > max_speed) right_speed = max_speed;
    if (right_speed < min_speed) right_speed = min_speed;

    if ((left_IR_value || right_IR_value) && (filter_boundary <= filtered_error && filtered_error <= filter_boundary)) {     // 직진
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
    } else if (left_IR_value && !right_IR_value) {     // 우회전
        motor_L.run(FORWARD); 
        motor_R.run(BACKWARD);     
    } else if (!left_IR_value && right_IR_value) {     // 좌회전 
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
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
void IRstate1 (unsigned long time_diff) {
    motor_L.run(RELEASE);
    motor_R.run(RELEASE);
    if(time_diff > 500) {
        time_previous = millis();
        obstacle_flag = 2;            
    }
}
// 좌로회전(장애물 감지 벗어날 때까지)
void IRstate2 () {
    motor_L.setSpeed(160);
    motor_R.setSpeed(160);
    motor_L.run(BACKWARD); 
    motor_R.run(FORWARD);
    time_previous = millis(); 
}
// 좌로 좀더 회전 -> 정지 -> 전진하면서 라인유무체크 -> 정지
void IRstate3 (unsigned long time_diff) {
    if(time_diff <= 200) {           // 200ms 좌회전
        motor_L.run(BACKWARD);
        motor_R.run(FORWARD);
    } else if(time_diff <= 700) {    // 500ms 정지
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    } else if(time_diff <= 1300)  {  // 500ms 전진
        motor_L.run(FORWARD);
        motor_R.run(FORWARD);
        left_IR_value = digitalRead(A0);
        right_IR_value = digitalRead(A3);
        if(left_IR_value == 1 || right_IR_value == 1) {
            obstacle_flag = 4;
            time_previous = millis();
        }
    } else if(time_diff <= 1800) {     // 500ms 정지
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    } else if(time_diff > 1800) {      // 다음 상태 진입
        obstacle_flag = 5;
    }
}
// 라인 발견 시 300ms 좌로 회전
void IRstate4 (unsigned long time_diff) {
    if(time_diff <= 500) {
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
    } else if(time_diff <= 800) {   
        motor_L.setSpeed(180);    
        motor_R.setSpeed(180);
        motor_L.run(BACKWARD);
        motor_R.run(FORWARD);
    } else if(time_diff > 800) {
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
}
void setup() {
    // IRPin set
    directSetPinIn(IRPin);  // IRPin을 입력으로 설정합니다.
    attachInterrupt(digitalPinToInterrupt(IRPin), obstacle_detected, CHANGE);

    // Motor set
    motor_L.setSpeed(max_speed);  motor_L.run(RELEASE);   // 좌측 모터 초기화 (속도 180, 전압 공급)
    motor_R.setSpeed(max_speed);  motor_R.run(RELEASE);   // 오른쪽 모터의 속도
    
    time_previous = millis();
    time_current = time_previous;
    time_lcdprevious = time_previous;

    // LCD set2
    lcd.init();  // LCD 초기화
    lcd.backlight();
    directSetPinIn(ONOFF);
    attachInterrupt(digitalPinToInterrupt(ONOFF), onoff, CHANGE);
}
void loop() {
    if (operation_state) {  // ON 상태
        time_current = millis();
        // LCD 클러스터
        if(time_current - time_lcdprevious >= 1000) {  // 1초에 한 번씩 동작
            time_lcdprevious = time_current;
            cluster();                             
        }
        time_current = millis();
        unsigned long time_diff = time_current - time_previous;
        // 장애물 회피
        switch(obstacle_flag) {
        case 0: car(); break;               // 일반 주행
        case 1: IRstate1(time_diff); break; // 500ms 정지
        case 2: IRstate2(); break;          // 장애물 인식 벗어나기 위한 좌회전 (방향 전환)
        case 3: IRstate3(time_diff); break; // 500ms 좌회전, 500ms 정지, 500ms 주행
        case 4: IRstate4(time_diff); break; // 좌회전
        case 5: IRstate5(); break;          // 장애물 인식 위한 우회전(방향 전환)
        }
    } else {  // OFF상태
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);
        lcd.clear();
        lcd.noBacklight();
    }
}