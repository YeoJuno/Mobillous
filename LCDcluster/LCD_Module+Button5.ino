#include <Wire.h>                  // i2C 통신을 위한 라이브러리
#include <LiquidCrystal_I2C.h>     // LCD 1602 I2C용 라이브러리

#define BUTTON 2
#define ONOFF 3

LiquidCrystal_I2C lcd(0x27, 16, 2); // 접근주소: 0x3F or 0x27

// 목적 
// 버튼 누르면 전원 off, distance velocity 초기화, display text clear
// 버튼 누르면 전원 on, display text on

bool init_flag=0;
bool onoff_flag=0;
int operation_State = 1;
bool mode = 1;
int distance = 100;
int velocity = 0;
int digit_distance = 12;
int digit_velocity = 9;
bool mode_tmp = 1;
int digit_distance_tmp = 0;
int digit_velocity_tmp = 0;
unsigned long time_previous, Time_Current;
int setcursor_tmp = 0;

String string_mode;
String string_distance;
String string_velocity;

void Lcd_Module();

void setup()
{
  lcd.init();  // LCD 초기화
  lcd.backlight();
  Serial.begin(9600);
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ONOFF), OnOff, CHANGE);
  time_previous = millis();  
}


int digitCheck(int num){  // 자릿수 체크
    if(num == 0) return 1;
    
    int cnt = 0;
    while(num!=0){
        num/=10;
        cnt++;
    }
    return cnt;
}

void writeLCD_distance(){  // 주행거리 입력
    setcursor_tmp = 13 - digit_distance;
    lcd.setCursor(setcursor_tmp,0);
    lcd.print(string_distance);
    lcd.setCursor(14,0);
    lcd.print("cm");        
}

void writeLCD_velocity(){   // 속도 입력
    setcursor_tmp = 10 - digit_velocity;
    lcd.setCursor(setcursor_tmp, 1);
    lcd.print(string_velocity);
    lcd.setCursor(11,1);
    lcd.print("cm/s");        
}

void writeLCD_mode(){   // mode 입력
    lcd.setCursor(1,0);
    if(mode){
        lcd.print("SPEED");
    }
    else {
        lcd.print("SAFETY");
    }
}

void cluster(){     // LCD cluster
    lcd.backlight();

    if(init_flag==1) lcd.clear();
    if(distance < 0) distance = 0;
    if(velocity < 0) velocity = 0;

    digit_distance = digitCheck(distance);  // 현재 distance 자릿 수를 저장
    digit_velocity = digitCheck(velocity);  // 현재 velocity 자릿 수를 저장
    
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

void OnOff(){
// onoff_flag
    if(digitalRead(ONOFF) == HIGH)
    {
        if(onoff_flag==0)
        {
            onoff_flag = 1;      
            operation_State = -operation_State;
            distance = 0;
            velocity = 0;
            // Serial.println(operation_State);
        }
        else{}
    }
    else
    {
        onoff_flag = 0;               
    }
}

void loop()
{   
    Time_Current = millis();
    if(operation_State == 1){   // ON 상태
        if(Time_Current - time_previous >= 1000){
            time_previous = Time_Current;
            cluster();
            Serial.println("timer");                             
        }
    }
    else if(operation_State == -1)
    {
        lcd.clear();
        lcd.noBacklight();
    }  // OFF 상태
}
