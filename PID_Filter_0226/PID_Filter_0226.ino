#include <mobis_io.h>
#include <mobis_motor.h>

Motor motor_L(3), motor_R(4);

const int num_samples = 100; // Number of samples to use for filtering
int sensor_data[num_samples]; // Array to store sensor data for filtering
int sensor_index = 0; // Index for storing sensor data in array

uint8_t max_speed = 250;   
uint8_t min_speed = 160;   
uint8_t speed_diff = 0;   
int kp = 204;   
int ki = 255;   
int kd = 2000;   
int last_error = 0;   
int error = 0;   
int total_error = 0;   
float total = 0;
float filter_val = 0.3;

void setup() {
    motor_L.setSpeed(max_speed);                
    motor_L.run(RELEASE);
    motor_R.setSpeed(max_speed);                 
    motor_R.run(RELEASE);
}

// Function to filter sensor data using a rolling average
float filterSensorData(int sensor_value) {
    total -= sensor_data[sensor_index];
    total += sensor_value;
    sensor_data[sensor_index] = sensor_value;
    sensor_index = (sensor_index + 1) % num_samples;
    return total / num_samples;
}


void loop() {
    int val1 = directDigitalRead(A0);      
    int val2 = directDigitalRead(A3);      
    int error1 = val1 - val2;

    // Filter sensor data using rolling average
    float filtered_error = filterSensorData(error1);
    //float filtered_error = error1;
    total_error += filtered_error;
    last_error = filtered_error; 
    int output = kp * filtered_error + ki * total_error + kd * (filtered_error - last_error);


    int left_speed = max_speed - 3* output;
    int right_speed = max_speed + 4* output;

    if (left_speed > max_speed) left_speed = max_speed;
    if (right_speed > max_speed) right_speed = max_speed;
    if (left_speed < min_speed) left_speed = min_speed;
    if (right_speed < min_speed) right_speed = min_speed;

    if ((filtered_error <= filter_val && filtered_error >= (-1) * filter_val) && (!(val1 == 0 && val2 == 0))) {
    // Both sensors on the line or filter average between 0.3 and -0.3 -> drive straight
        motor_L.setSpeed(max_speed);
        motor_R.setSpeed(max_speed);
        motor_L.run(FORWARD); 
        motor_R.run(FORWARD);
  }
    else if (val1 == 1 && val2 == 0) {
    // Left sensor on the line, turn right
        motor_L.setSpeed(left_speed - speed_diff);
        motor_R.setSpeed(min_speed);
        motor_L.run(FORWARD); 
        motor_R.run(BACKWARD);
  }
     else if (val1 == 0 && val2 == 1){
    // Right sensor on the line, turn left
        motor_L.setSpeed(min_speed);
        motor_R.setSpeed(right_speed - speed_diff);
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
 
  }
}