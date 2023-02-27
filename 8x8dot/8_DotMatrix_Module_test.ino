#include "LedControl.h"
LedControl lc=LedControl(13,9,10,1);
                        
byte light_off = {B00000000};
byte light_on = {B11111111};
int turn_intensity = 0;  
int turn_intensity_tmp = 0;  
unsigned long light_time_previous, light_time_current;
int light_count=0;
bool light_onoff_flag = true;
int turn_state=0;  // -1 좌, 0 직진, 1 우
int turn_state_tmp=0;  // 100ms 이전 turn_state 저장
int test_val=0;
int test_flag=1;


void setup(){
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

void TEST(){
    if(test_val > 20){
        turn_state = -1;
        test_flag = -1;
    }
    else if(test_val < -20){
        turn_state = 1;
        test_flag = 1;
    }
}

void loop(){
   
    light_time_current = millis();

    if(light_time_current - light_time_previous >= 50){
        light_time_previous = light_time_current;

        TEST();
        test_val += test_flag;        

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
