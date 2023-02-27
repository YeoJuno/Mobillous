#ifndef _MOBIS_MOTOR_H_
#define _MOBIS_MOTOR_H_

/*  74HCT595 : SIPO(Serial Input Parallel Output) Shift Register
		Q0~Q7: Output
		OE: Output Enable
		DS: Data input pin which serial data come in
		ST_cp: Storage(Latch) Register Clock Pin (Store received serial data to Latch register.)
		SH_cp: Shift Register Clock Pin (Synchonize between master and slave.)
		MR: Master Re-clear == Reset Pin (Active low) 	*/
#define MOTOR_CLK        4
#define MOTOR_ENABLE     7
#define MOTOR_DATA       8
#define MOTOR_LATCH      12

/* Bit positions in the 74HCT595 shift register output
   Latch Data == Shift register input value
  ┌----------------------------------┐
  |  [7] [6] [5] [4] [3] [2] [1] [0] |
  |  3B  4B  3A  2B  1B  1A  2A  4A  |
  └----------------------------------┘ */
#define MOTOR1_A        2
#define MOTOR1_B        3
#define MOTOR2_A        1
#define MOTOR2_B        4
#define MOTOR3_A        5
#define MOTOR3_B        7
#define MOTOR4_A        0
#define MOTOR4_B        6

/* Constants that the user passes in to the motor calls */
#define FORWARD         1
#define BACKWARD        2
#define BRAKE           3
#define RELEASE         4

/* TCCR2B - TCCR0B - Timer/Counter Control Register B for Timer2 */
#define MOTOR12_64KHZ   bit(0)              // No prescale
#define MOTOR12_8KHZ    bit(1)              // divide by 8
#define MOTOR12_2KHZ    bit(1) | bit(0)     // divide by 32
#define MOTOR12_1KHZ    bit(2)              // divide by 64
/*
TCCR0B - Timer/Counter Control Register B for Timer0
    [7]:	FOC0A(Force Output Compare A)
    [6]:	FOC0B(Force Output Compare B)
    [5:4]:	Reserved
    [3]:	WGM02(Waveform Generation Mode)
    [2:0]:	CS(Clock Source select bits)
				000: No clk source	001: 1:1	010: 1:8    011: 1:64
*/
#define MOTOR34_64KHZ   bit(0)              // No prescale
#define MOTOR34_8KHZ    bit(1)              // Divide by 8
#define MOTOR34_1KHZ    bit(1) | bit(0)     // Divide by 64

#define PWM1_PIN        11
#define PWM2_PIN        3
#define PWM3_PIN        6
#define PWM4_PIN        5


class MotorController {
public:
	void enableMotor();
	void xferDataToShifter();
};
class Motor : public MotorController{
	uint8_t motorNum;
	uint8_t motorFreq;
	uint8_t motorSpeed;
public:
	// 30Hz <= Arduino PWM frequency <= 65Khz
	Motor(uint8_t channel, uint8_t frequency = MOTOR34_64KHZ);
	void run(uint8_t command);
	void setSpeed(uint8_t speed);
    // Use PWM from timer2A on PB3 (Arduino Uno pin 11)
    void initPWM1(uint8_t frequency);
    void setPWM1(uint8_t timerLimitValue) { (*(volatile uint8_t *)(0xB3)) = timerLimitValue; }
    // Use PWM from timer2B on PD3 (Arduino Uno pin 3)
    void initPWM2(uint8_t frequency);
    void setPWM2(uint8_t timerLimitValue) { (*(volatile uint8_t *)(0xB4)) = timerLimitValue; }
    // Use PWM from timer0A on PD6 (Arduino Uno pin 6)
    void initPWM3(uint8_t frequency);
    void setPWM3(uint8_t timerLimitValue) { (*(volatile uint8_t *)(0x47)) = timerLimitValue; }
    // Use PWM from timer0B on PD5 (Arduino Uno pin 5)
    void initPWM4(uint8_t frequency);
    void setPWM4(uint8_t timerLimitValue) { (*(volatile uint8_t *)(0x48)) = timerLimitValue; }
};

#endif