#include "Arduino.h"
#include "mobis_io.h"
#include "mobis_motor.h"

static MotorController MC;
static uint8_t latchData;

void MotorController::enableMotor() {
	latchData = 0;
    directSetPinOut(MOTOR_LATCH);
    directSetPinOut(MOTOR_ENABLE);
    directSetPinOut(MOTOR_DATA);
    directSetPinOut(MOTOR_CLK);
	
    xferDataToShifter();

	directDigitalWrite(MOTOR_ENABLE, LOW);  // OE pin is active low.
}

void MotorController::xferDataToShifter() {
    /* To overwrite Latch register, we should make SH_cp Low to High.
        It is vise versa in data register. */
    directDigitalWrite(MOTOR_LATCH, LOW);
    directDigitalWrite(MOTOR_DATA, LOW);

	for (uint8_t i = 0; i < 8; ++i) {
		directDigitalWrite(MOTOR_CLK, LOW);
		if (latchData & bit(7 - i)) directDigitalWrite(MOTOR_DATA, HIGH);
        else directDigitalWrite(MOTOR_DATA, LOW);
		directDigitalWrite(MOTOR_CLK, HIGH);
	}
	digitalWrite(MOTOR_LATCH, HIGH);
}

Motor::Motor(uint8_t channel, uint8_t frequency = MOTOR34_64KHZ) {
    this->motorNum = channel;
    this->motorFreq = frequency;

    MC.enableMotor();

    switch (channel) {
    case 1:
        /* For motor wired with channel 1 */
        latchData &= ~(bit(MOTOR1_A) | bit(MOTOR1_B));
        MC.xferDataToShifter();
        initPWM1(frequency);
        break;
    case 2:
        /* For motor wired with channel 2 */
        latchData &= ~(bit(MOTOR2_A) | bit(MOTOR2_B));
        MC.xferDataToShifter();
        initPWM2(frequency);
        break;
    case 3:
        /* For motor wired with channel 3 */
        latchData &= ~(bit(MOTOR3_A) | bit(MOTOR3_B));
        MC.xferDataToShifter();
        initPWM3(frequency);
        break;
    case 4:
        /* For motor wired with channel 4 */
        latchData &= ~(bit(MOTOR4_A) | bit(MOTOR4_B));
        MC.xferDataToShifter();
        initPWM4(frequency);
        break;
    }
}

/* Initialize PWM setting for Timer2A */
void Motor::initPWM1(uint8_t frequency) {
    /* Config TCCR2A : Fast PWM mode + Clear OC2A on compare match + Count from bottom */
    (*(volatile uint8_t *)(0xB0)) |= (bit(7) | bit(1) | bit(0));
    /* Config TCCR2B : No clock prescale at all, 1:1 (Because of CS22, CS21, CS20 = 0b001) */
    (*(volatile uint8_t *)(0xB1)) |= (this->motorFreq & 0x7);
    /* Clear OCR2A(0xB3) to 0 */
    (*(volatile uint8_t *)(0xB3)) = 0;
    directSetPinOut(PWM1_PIN);
}
/* Initialize PWM setting for Timer2B */
void Motor::initPWM2(uint8_t frequency) {
    /* Config TCCR2A(0xB0) : Fast PWM mode + Clear OC2B on compare match + Count from bottom */
    (*(volatile uint8_t *)(0xB0)) |= (bit(5) | bit(1) | bit(0));
    /* Config TCCR2B(0xB1) : No clock prescale at all, 1:1 (Because of CS22, CS21, CS20 = 0b001) */
    (*(volatile uint8_t *)(0xB1)) |= (this->motorFreq & 0x7);
    /* Clear OCR2B(0xB4) to 0 */
    (*(volatile uint8_t *)(0xB4)) = 0;
    directSetPinOut(PWM2_PIN);
}
/* Initialize PWM setting for Timer0A */
void Motor::initPWM3(uint8_t frequency) {
    /* Config TCCR0A(0x44) : Fast PWM mode + Clear OC0A on compare match + Count from bottom */
    (*(volatile uint8_t *)(0x44)) |= (bit(7) | bit(1) | bit(0));
    /* Config TCCR0B(0x45) : No clock prescale at all, 1:1 (Because of CS02, CS01, CS00 = 0b001) */
    (*(volatile uint8_t *)(0x45)) |= (this->motorFreq & 0x7);
    /* Clear OCR0A(0x47) to 0 */
    (*(volatile uint8_t *)(0x47)) = 0;
    directSetPinOut(PWM3_PIN);
}
/* Initialize PWM setting for Timer0B */
void Motor::initPWM4(uint8_t frequency) {
    /* Config TCCR0A(0x44) : Fast PWM mode + Clear OC0B on compare match + Count from bottom */
    (*(volatile uint8_t *)(0x44)) |= (bit(5) | bit(1) | bit(0));
    /* Config TCCR0B(0x45) : No clock prescale at all, 1:1 (Because of CS02, CS01, CS00 = 0b001) */
    (*(volatile uint8_t *)(0x45)) |= (this->motorFreq & 0x7);
    /* Clear OCR0B(0x48) to 0 */
    (*(volatile uint8_t *)(0x48)) = 0;
    directSetPinOut(PWM4_PIN);
}

void Motor::run(uint8_t command) {
    uint8_t leftMotor, rightMotor;
    switch (this->motorNum) {
    case 1: leftMotor = MOTOR1_A; rightMotor = MOTOR1_B; break;
    case 2: leftMotor = MOTOR2_A; rightMotor = MOTOR2_B; break;
    case 3: leftMotor = MOTOR3_A; rightMotor = MOTOR3_B; break;
    case 4: leftMotor = MOTOR4_A; rightMotor = MOTOR4_B; break;
    }
    switch (command) {
    case FORWARD:
        _BIT_SET_(latchData, leftMotor);
        _BIT_UNSET_(latchData, rightMotor);
        break;
    case BACKWARD:
        _BIT_UNSET_(latchData, leftMotor);
        _BIT_SET_(latchData, rightMotor);
        break;
    case RELEASE:
        _BIT_UNSET_(latchData, leftMotor);
        _BIT_UNSET_(latchData, rightMotor);
        break;        
    }
    MC.xferDataToShifter();
}

void Motor::setSpeed(uint8_t speed) {
    switch (this->motorNum) {
    case 1: setPWM1(speed); break;
    case 2: setPWM2(speed); break;
    case 3: setPWM3(speed); break;
    case 4: setPWM4(speed); break;
    }
}