#include <SoftwareSerial.h>
#include <AFMotor.h>

AF_DCMotor motor_L(3);                
AF_DCMotor motor_R(4); 

int max_speed = 255;   
int min_speed = 150;   
int speed_diff = 60;   
int Kp = 200;   
int Ki = 0.2;   
int Kd = 0.5;   
int last_error = 0;   
int error = 0;   
int total_error = 0;   

void setup() {
  Serial.begin(9600);                
  Serial.println("Eduino Smart Car Start!");

    
  motor_L.setSpeed(max_speed);                
  motor_L.run(RELEASE);
  motor_R.setSpeed(max_speed);                 
  motor_R.run(RELEASE);
}

void loop() {
    int val1 = digitalRead(A0);      
    int val2 = digitalRead(A5);      

    int a = 0;
    error = val1 - val2 + 0.1;

      
    total_error += error;

      
    int output = Kp * error + Ki * total_error + Kd * (error - last_error);

      
    last_error = error;

      
    int left_speed = max_speed - 5* output;
    int right_speed = max_speed + 5*output;

      
    if (left_speed > max_speed) left_speed = max_speed;
    if (right_speed > max_speed) right_speed = max_speed;
    if (left_speed < min_speed) left_speed = min_speed;
    if (right_speed < min_speed) right_speed = min_speed;

      
    if (val1 == 1 && val2 == 1) {                     
       motor_L.setSpeed(max_speed);
       motor_R.setSpeed(max_speed);
       motor_L.run(FORWARD); 
       motor_R.run(FORWARD);
    }
    else if (val1 == 1 && val2 == 0) {                
       motor_L.setSpeed(left_speed - speed_diff);
       motor_R.setSpeed(min_speed - a);
       motor_L.run(FORWARD); 
       motor_R.run(BACKWARD);
  
    }
    else if (val1 == 0 && val2 == 1) {                
       motor_L.setSpeed(min_speed - a);
       motor_R.setSpeed(right_speed - speed_diff);
       motor_L.run(BACKWARD); 
       motor_R.run(FORWARD);
      
    } 
    else if (val1 == 0 && val2 == 0) {                
       //motor_L.setSpeed(min_speed+ a);
      // motor_R.setSpeed(min_speed+ a);
      // motor_L.run(RELEASE); 
      // motor_R.run(RELEASE);
    }
   
}