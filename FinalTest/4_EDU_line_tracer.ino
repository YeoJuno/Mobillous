#define _BIT_(x)                    ( 1 << (x) )
#define _BIT_RESET_(x)              ( (x) = 0x00 )
#define _BIT_SET_(x, pos)           ( (x) |= (_BIT_(pos)) )
#define _BIT_TOGGLE_(x, pos)        ( (x) ^= (_BIT_(pos)) )
#define _BIT_UNSET_(x, pos)         ( (x) &= (~(_BIT_(pos))) )
#define _BIT_CHECK_(x, pos)         ( ((x) >> (pos)) & 0x01 )
#define _BIT_WRITE_(x, pos, data)   ( (data) ? ((_BIT_SET_((x), (pos)))) : ((_BIT_UNSET_((x), (pos)))) )

typedef struct UnoPin {
    volatile uint8_t* addrPort;    // Address of PORTx.
    volatile uint8_t* addrDdr;     // Address of DDRx.
    volatile uint8_t* addrPin;     // Address of PINx.
    uint8_t pinNumInPort;          // Pin number within the port.
} UnoPin;

static const UnoPin unoPin[] = {
    /* ------ Port D (Digital 0 ~ 7) ------- */
    {0x2B, 0x2A, 0x29, 0}, {0x2B, 0x2A, 0x29, 1}, {0x2B, 0x2A, 0x29, 2},
    {0x2B, 0x2A, 0x29, 3}, {0x2B, 0x2A, 0x29, 4}, {0x2B, 0x2A, 0x29, 5},
    {0x2B, 0x2A, 0x29, 6}, {0x2B, 0x2A, 0x29, 7},
    /* ------ Port B (Digital 8 ~ 13) ------- */
    {0x25, 0x24, 0x23, 0}, {0x25, 0x24, 0x23, 1}, {0x25, 0x24, 0x23, 2},
    {0x25, 0x24, 0x23, 3}, {0x25, 0x24, 0x23, 4}, {0x25, 0x24, 0x23, 5},
    /* ------ Port C (Analog 0 ~ 5) ------- */
    {0x28, 0x27, 0x26, 0}, {0x28, 0x27, 0x26, 1}, {0x28, 0x27, 0x26, 2},
    {0x28, 0x27, 0x26, 3}, {0x28, 0x27, 0x26, 4}, {0x28, 0x27, 0x26, 5}
};

inline void directSetPinOut(uint8_t pin) {
    _BIT_SET_(*(unoPin[pin].addrDdr), unoPin[pin].pinNumInPort);
}
inline void directSetPinIn(uint8_t pin) {
    _BIT_UNSET_(*(unoPin[pin].addrDdr), unoPin[pin].pinNumInPort);
    _BIT_UNSET_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}
inline uint8_t directDigitalRead(uint8_t pin) {
    return _BIT_CHECK_(*(unoPin[pin].addrPin), unoPin[pin].pinNumInPort);
}
inline void directDigitalWrite(uint8_t pin, uint8_t data) {
    _BIT_WRITE_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort, data);
}
inline void directDigitalToggle(uint8_t pin) {
    _BIT_TOGGLE_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}

/* ==================== Motor ======================= */
/* 
74HCT595 : SIPO(Serial Input Parallel Output) Shift Register
    Q0~Q7: Output
    OE: Output Enable
    DS: Data input pin which serial data come in
    ST_cp: Storage(Latch) Register Clock Pin (Store received serial data to Latch register.)
    SH_cp: Shift Register Clock Pin (Synchonize between master and slave.)
    MR: Master Re-clear == Reset Pin (Active low)
*/
#define MOTOR_CLK        4
#define MOTOR_ENABLE     7
#define MOTOR_DATA       8
#define MOTOR_LATCH      12

// Bit positions in the 74HCT595 shift register output
/* Latch Data == Shift register input value
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

// Constants that the user passes in to the motor calls
#define FORWARD         1
#define BACKWARD        2
#define BRAKE           3
#define RELEASE         4

/*
TCCR2B - TCCR0B - Timer/Counter Control Register B for Timer2
*/
#define MOTOR12_64KHZ   bit(0)              // No prescale
#define MOTOR12_8KHZ    bit(1)              // divide by 8
#define MOTOR12_2KHZ    bit(1) | bit(0)     // divide by 32
#define MOTOR12_1KHZ    bit(2)              // divide by 64
/*
TCCR0B - Timer/Counter Control Register B for Timer0
    [7]: FOC0A(Force Output Compare A)
    [6]: FOC0B(Force Output Compare B)
    [5:4]: Reserved
    [3]: WGM02(Waveform Generation Mode)
    [2:0]: CS(Clock Source select bits)
        000: No clock source    001: 1:1    010: 1:8    011: 1:64
*/
#define MOTOR34_64KHZ   bit(0)              // No prescale
#define MOTOR34_8KHZ    bit(1)              // Divide by 8
#define MOTOR34_1KHZ    bit(1) | bit(0)     // Divide by 64

#define PWM1_PIN        11
#define PWM2_PIN        3
#define PWM3_PIN        6
#define PWM4_PIN        5

static uint8_t latchData;

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

static MotorController MC;

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

// Initialize PWM setting for Timer2A
void Motor::initPWM1(uint8_t frequency) {
    // Config TCCR2A : Fast PWM mode + Clear OC2A on compare match + Count from bottom
    (*(volatile uint8_t *)(0xB0)) |= (bit(7) | bit(1) | bit(0));
    // Config TCCR2B : No clock prescale at all, 1:1 (Because of CS22, CS21, CS20 = 0b001)
    (*(volatile uint8_t *)(0xB1)) |= (this->motorFreq & 0x7);
    // Clear OCR2A(0xB3) to 0
    (*(volatile uint8_t *)(0xB3)) = 0;
    directSetPinOut(PWM1_PIN);
}
// Initialize PWM setting for Timer2B
void Motor::initPWM2(uint8_t frequency) {
    // Config TCCR2A(0xB0) : Fast PWM mode + Clear OC2B on compare match + Count from bottom
    (*(volatile uint8_t *)(0xB0)) |= (bit(5) | bit(1) | bit(0));
    // Config TCCR2B(0xB1) : No clock prescale at all, 1:1 (Because of CS22, CS21, CS20 = 0b001)
    (*(volatile uint8_t *)(0xB1)) |= (this->motorFreq & 0x7);
    // Clear OCR2B(0xB4) to 0
    (*(volatile uint8_t *)(0xB4)) = 0;
    directSetPinOut(PWM2_PIN);
}
// Initialize PWM setting for Timer0A
void Motor::initPWM3(uint8_t frequency) {
    // Config TCCR0A(0x44) : Fast PWM mode + Clear OC0A on compare match + Count from bottom
    (*(volatile uint8_t *)(0x44)) |= (bit(7) | bit(1) | bit(0));
    // Config TCCR0B(0x45) : No clock prescale at all, 1:1 (Because of CS02, CS01, CS00 = 0b001)
    (*(volatile uint8_t *)(0x45)) |= (this->motorFreq & 0x7);
    // Clear OCR0A(0x47) to 0
    (*(volatile uint8_t *)(0x47)) = 0;
    directSetPinOut(PWM3_PIN);
}
// Initialize PWM setting for Timer0B
void Motor::initPWM4(uint8_t frequency) {
    // Config TCCR0A(0x44) : Fast PWM mode + Clear OC0B on compare match + Count from bottom
    (*(volatile uint8_t *)(0x44)) |= (bit(5) | bit(1) | bit(0));
    // Config TCCR0B(0x45) : No clock prescale at all, 1:1 (Because of CS02, CS01, CS00 = 0b001)
    (*(volatile uint8_t *)(0x45)) |= (this->motorFreq & 0x7);
    // Clear OCR0B(0x48) to 0
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

Motor motor_L(3), motor_R(4);

void setup() {
  // turn on motor
  motor_L.setSpeed(255);              // 왼쪽 모터의 속도   
  motor_L.run(RELEASE);
  motor_R.setSpeed(255);              // 오른쪽 모터의 속도   
  motor_R.run(RELEASE);
}

void loop() {
    int val1 = directDigitalRead(A0);    // 라인센서1
    int val2 = directDigitalRead(A3);    // 라인센서2   
    
      if (val1 == 1 && val2 == 1) {                   // 직진
       motor_L.setSpeed(255);
       motor_R.setSpeed(255);
       motor_L.run(FORWARD); 
       motor_R.run(FORWARD);
    //    Serial.println("F");
      }
      else if (val1 == 0 && val2 == 1) {              // 우회전 1
       motor_R.setSpeed(105); 
       motor_L.run(FORWARD); 
       motor_R.run(BACKWARD);
    //    Serial.println("->");       
      }
      else if (val1 == 1 && val2 == 0) {              // 좌회전 2
        motor_L.setSpeed(105);
        motor_L.run(BACKWARD); 
        motor_R.run(FORWARD);
        // Serial.println("<-");
      } 
      else if (val1 == 0 && val2 == 0) {              // 정지
        // motor_R.setSpeed(160);
        // motor_L.setSpeed(160);
      }           
}
    
