#include <mobis_io.h>
#include <mobis_motor.h>

Motor motor_L(3), motor_R(4);

void setup() {
    directSetPinIn(A0);
    directSetPinIn(A3);
    motor_L.setSpeed(255);              // 왼쪽 모터의 속도   
    motor_L.run(RELEASE);
    motor_R.setSpeed(255);              // 오른쪽 모터의 속도   
    motor_R.run(RELEASE);
}

void loop() {
    uint8_t val1 = directDigitalRead(A0);
    uint8_t val2 = directDigitalRead(A3); 
	/* 직진 */
    if (val1 == 1 && val2 == 1) {
        motor_L.setSpeed(255);
        motor_R.setSpeed(255);
        motor_L.run(FORWARD); 
        motor_R.run(FORWARD);
    }
	/* 우회전 */
    else if (val1 == 0 && val2 == 1) {
        motor_R.setSpeed(105); 
        motor_L.run(FORWARD); 
        motor_R.run(BACKWARD);
    }
	/* 좌회전 */
    else if (val1 == 1 && val2 == 0) {
        motor_L.setSpeed(105);
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
    } 
}
