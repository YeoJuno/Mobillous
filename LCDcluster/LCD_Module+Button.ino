#include <Wire.h>                  // i2C 통신을 위한 라이브러리
#include <LiquidCrystal_I2C.h>     // LCD 1602 I2C용 라이브러리

#define BUTTON 2

LiquidCrystal_I2C lcd(0x27, 16, 2); // 접근주소: 0x3F or 0x27

bool btn_flag=0;
bool mode = 1;
int distance = 100;
int velocity = 0;
int digit_distance = 12;
int digit_velocity = 9;
bool mode_tmp = 1;
int digit_distance_tmp = 0;
int digit_velocity_tmp = 0;

String string_mode;
String string_distance;
String string_velocity;

void Lcd_Module();

void setup()
{
  lcd.init();  // LCD 초기화
  lcd.backlight();
  Serial.begin(9600);
  pinMode(2,INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), cluster_Reset, HIGH);
}

void LCD_reset(){
    lcd.clear();
    distance = 0;
    velocity = 0;
}

int digitCheck(int num){
    int cnt = 0;
    while(num!=0){
        num/=10;
        cnt++;
    }
    return cnt;
}

void writeLCD_distance(){
    if(digit_distance <= 1){
        lcd.setCursor(12,0);
        lcd.print(string_distance);
    }
    else if(digit_distance == 2){
        lcd.setCursor(11,0);
        lcd.print(string_distance);
    }
    else if(digit_distance == 3){
        lcd.setCursor(10,0);
        lcd.print(string_distance);
    }
    else if(digit_distance == 4){
        lcd.setCursor(9,0);
        lcd.print(string_distance);
    }
    lcd.setCursor(14,0);
    lcd.print("cm");        
}

void writeLCD_velocity(){
    if(digit_velocity <= 1){
        lcd.setCursor(9,1);
        lcd.print(string_velocity);
    }
    else if(digit_velocity == 2){
        lcd.setCursor(8,1);
        lcd.print(string_velocity);
    }
    else if(digit_velocity == 3){
        lcd.setCursor(8,1);
        lcd.print(string_velocity);
    }
    lcd.setCursor(11,1);
    lcd.print("cm/s");        
}

void writeLCD_mode(){
    lcd.setCursor(1,0);
    if(mode){
        lcd.print("SPEED");
    }
    else {
        lcd.print("SAFETY");
    }
}

void cluster(){
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

    writeLCD_distance();
    writeLCD_velocity();
    writeLCD_mode();

    delay(200);
    distance += 30;
    velocity += 3;
    mode = !mode;

    digit_distance_tmp = digit_distance;
    digit_velocity_tmp = digit_velocity;
    mode_tmp = mode;
    
    lcd.clear();    
}

void cluster_Reset(){
    if(btn_flag==0){
        btn_flag = 1;
        Serial.println("===PUSHED===");
    }
}

void loop()
{
//   lcd.clear();                 // 모든 문자 지우기
//   lcd.setCursor(col, row);
//   lcd.backlight();                // 백라이트 켜기
//   lcd.setCursor(0, 0);            // 1번째, 1라인
//   lcd.print("Hello world!");

    cluster();
    // if(digitalRead(BUTTON) == HIGH){
    //     Serial.println("HIGH");
    // }
    // else{
    //     Serial.println("LOW");
    // }
    btn_flag = 0;
    Serial.println("=====RELEASED====");    
}
