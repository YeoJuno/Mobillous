#include <SoftwareSerial.h>
#include <AFMotor.h>

AF_DCMotor motor_L(1);              // 모터드라이버 L293D  1: M1에 연결,  4: M4에 연결
AF_DCMotor motor_R(2);


//

double Kp = 0.2;
double Kd = 0.2;
double Ki = 0.1;

//

int error = 0;
double lastError = 0;
double I_err = 0;
double D_err = 0;
double setPoint = 0;
double output = 0;

int motorSpeed = 200;
int leftMotorSpeed = motorSpeed;
int rightMotorSpeed = motorSpeed;

void setup() {

  Serial.begin(9600);
  Serial.println("Eduino Smart Car Start!");

 
  motor_L.setSpeed(leftMotorSpeed);
  motor_R.setSpeed(rightMotorSpeed);
}

void loop() {
  int sensorVal1 = digitalRead(A0);
  int sensorVal2 = digitalRead(A5);

    
  error = sensorVal1 - sensorVal2;
  I_err += error;
  D_err = error - lastError;


  output = Kp * error + Ki * I_err + Kd * D_err;

  leftMotorSpeed = motorSpeed + output;
  rightMotorSpeed = motorSpeed - output;

/*
  <Set Speed Threshold>
*/

  if (leftMotorSpeed > 255) leftMotorSpeed = 255;
  if (leftMotorSpeed < 150) leftMotorSpeed = 150;
  if (rightMotorSpeed > 255) rightMotorSpeed = 255;
  if (rightMotorSpeed < 150) rightMotorSpeed = 150;




  motor_L.run(leftMotorSpeed >= motorSpeed ? FORWARD : BACKWARD);
  motor_R.run(rightMotorSpeed >= motorSpeed ? FORWARD : BACKWARD);
  motor_L.setSpeed(leftMotorSpeed);
  motor_R.setSpeed(rightMotorSpeed);


  lastError = error;
}


/*

P, I, D 게인 값 조정 필요 (P, I, D)

초기값 : (0.1, 0.1, 0.1)

*/