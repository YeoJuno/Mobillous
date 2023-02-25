#include <AFMotor.h>

#define LEFT_STATE 1
#define RIGHT_STATE 2

#define BIT(pos)                        ( 1 << (pos) )
#define CHECK_BIT(x, pos)               ( ( 0u == ( (x) & (BIT(pos)) ) ) ? 0u : 1u )
#define SET_BIT(x, pos)                 ( (x) |= (BIT(pos)) )
#define SET_BITS(x, bits)               ( (x) |= (bits) )
#define UNSET_BIT(x, pos)               ( (x) &= ~(BIT(pos)) )
#define UNSET_BITS(x, bits)             ( (x) &= (~(bits)) )
#define FLIP_BIT(x, pos)                ( (x) ^= (BIT(pos)) )
#define FLIP_BITS(x, bits)              ( (x) ^= (bits) )
#define GET_BITFIELD(x, start, len)     ( ((x) >> (start)) & (BIT(len) - 1) )


AF_DCMotor motor_L(1, 0);                  // 모터드라이버 L293D
AF_DCMotor motor_R(2, 0); 

int state = 1;                     // 0 = 직진, 1 = 우회전 2 = 좌회전
int val1, val2;

void setup() {
    pinMode(A0, INPUT);
    pinMode(A5, INPUT);
    // turn on motor
    motor_L.setSpeed(255);              // 왼쪽 모터의 속도   
    motor_L.run(RELEASE);
    motor_R.setSpeed(255);              // 오른쪽 모터의 속도   
    motor_R.run(RELEASE);
}

void loop() {
        val1 = digitalRead(A0);    // 라인센서1
        val2 = digitalRead(A5);    // 라인센서2   
    
        if (val1 == 0 && val2 == 0) {                   // 직진
            motor_L.run(FORWARD); 
            motor_R.run(FORWARD);
        }
        else if (val1 == 0 && val2 == 1) {              // 우회전
            motor_L.run(FORWARD); 
            motor_R.run(BACKWARD);
            state = RIGHT_STATE;
        }
        else if (val1 == 1 && val2 == 0) {              // 좌회전
            motor_L.run(BACKWARD); 
            motor_R.run(FORWARD);
            state = LEFT_STATE;
        } 
        else {              // 트랙 벗어남
            if (state == LEFT_STATE) {
                motor_L.run(RELEASE); 
                motor_R.run(FORWARD);
            } else {
                motor_L.run(FORWARD); 
                motor_R.run(RELEASE);
            }       
        }           
      
}