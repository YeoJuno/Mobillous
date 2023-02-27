#include <mobis_motor.h>
#include <mobis_timer.h>
#include <mobis_dotmatrix.h>
#include <mobis_io.h>
#include <SoftwareSerial.h>
#include <AFMotor.h>

AF_DCMotor motor_L(3);                
AF_DCMotor motor_R(4); 

int max_speed = 250;   
int min_speed = 150;   
int speed_diff = 30;   
int Kp = 20;   
int Ki = 0.4;   
int Kd = 0.5;   
int last_error = 0;   
int error = 0;   
int total_error = 0;   
float total = 0;
float filter = 0;
float previous = 0;
float filter_val = 0.05;
float sensor_reading[200];
int sensor_index = 0;

void setup() {
  Serial.begin(9600);                
  Serial.println("Eduino Smart Car Start!");

    
  motor_L.setSpeed(max_speed);                
  motor_L.run(RELEASE);
  motor_R.setSpeed(max_speed);                 
  motor_R.run(RELEASE);
}


float weighted_average(float new_value) {
  float sum = 0;
  float weight = 1;
  for (int i = 0; i < 100; i++) {
    sum += sensor_reading[i] * weight;
    weight -= filter_val;
  }
  sum += new_value * weight;
  sensor_reading[sensor_index++] = new_value;
  if (sensor_index >= 100) sensor_index = 0;
  return sum / 2;
}
inline float map1(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
    int val1 = digitalRead(A0);      
    int val2 = digitalRead(A3);      
    int a = 0;
    int error1 = val1 - val2;

    if ( !(val1 == 0 && val2 == 0)) filter = weighted_average(error1);

    //error = filter;
    total_error += error;
    last_error = error; 
    int output = Kp * error + Ki * total_error + Kd * (error - last_error);
    
    int left_speed = max_speed - 5* output;
    int right_speed = max_speed + 5*output;

    if (left_speed > max_speed) left_speed = max_speed;
    if (right_speed > max_speed) right_speed = max_speed;
    if (left_speed < min_speed) left_speed = min_speed;
    if (right_speed < min_speed) right_speed = min_speed;

   /*   
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
*/
    filter = map1(filter, -1, 1, -0.5 , 0.5);

    if ((filter <=filter_val && filter >=(-1) * filter_val) && (!(val1 == 0 && val2 == 0)) ){
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

    else if (val1 == 0 && val2 == 1){
        motor_L.setSpeed(min_speed - a);
        motor_R.setSpeed(right_speed - speed_diff);
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
    }
  
    else {}
    Serial.println(filter);
}