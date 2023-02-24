/* 이 소스는 에듀이노(Eduino)에 의해서 번역, 수정, 작성되었고 소유권 또한 에듀이노의 것입니다. 
 *  소유권자의 허락을 받지 않고 무단으로 수정, 삭제하여 배포할 시 법적인 처벌을 받을 수도 있습니다. 
 *  
 *  에듀이노 교육용 2휠 라인트레이서 기본예제 
 *  
 */

#include <SoftwareSerial.h>
#include <AFMotor.h>
AF_DCMotor motor_L(1);              // 모터드라이버 L293D  1: M1에 연결,  4: M4에 연결
AF_DCMotor motor_R(2); 
int turn_state = 0;                 // 0 = 직진, 1 = 우회전 2 = 좌회전

void setup() {
  Serial.begin(9600);              // PC와의 시리얼 통신속도
  Serial.println("Eduino Smart Car Start!");

  // turn on motor
  motor_L.setSpeed(255);              // 왼쪽 모터의 속도   
  motor_L.run(RELEASE);
  motor_R.setSpeed(255);              // 오른쪽 모터의 속도   
  motor_R.run(RELEASE);
}

void loop() {
    int val1 = digitalRead(A0);    // 라인센서1
    int val2 = digitalRead(A5);    // 라인센서2   
    
      if (val1 == 0 && val2 == 0) {                   // 직진 0
       motor_L.setSpeed(255);
       motor_R.setSpeed(255);
       motor_L.run(FORWARD); 
       motor_R.run(FORWARD);
       //Serial.println("F");
       turn_state = 0;
      }
      else if (val1 == 0 && val2 == 1) {              // 우회전 1
       motor_L.setSpeed(60);
       motor_R.setSpeed(255);
       motor_L.run(FORWARD);
       motor_R.run(BACKWARD);
       //Serial.println("->");
       turn_state = 1;
      }
      else if (val1 == 1 && val2 == 0) {              // 좌회전 2
        motor_L.setSpeed(255);                                                                                                                                                                                                                                                 
        motor_L.run(BACKWARD); 
        motor_R.setSpeed(60);
        motor_R.run(FORWARD);
        //Serial.println("<-");
        turn_state = 1;
      } 
      else if (val1 == 1 && val2 == 1 && turn_state == 0) {              // 정지
        // motor_L.setSpeed(150);
        // motor_R.setSpeed(150);
        // motor_L.run(BACKWARD); 
        // motor_R.run(BACKWARD);
        // motor_L.run(RELEASE);
        // motor_R.run(RELEASE);
        //Serial.println("s");
        motor_L.run(RELEASE);
        motor_R.run(RELEASE);            
      }                
}