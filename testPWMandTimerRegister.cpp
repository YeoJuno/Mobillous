#define _BV(bit) (1 << (bit))

/*
MOTORLATCH: Digital 12번 핀
MOTORENABLE: Digital 7번 핀
MOTORDATA: Digital 8번 핀
MOTORCLK: Digital 4번 핀
*/

static AFMotorController MC;
static uint8_t latch_state

void AFMotorController::latch_tx(void) {
	uint8_t i;
	digitalWrite(MOTORLATCH, LOW);		// LATCH_PORT &= ~_BV(LATCH);
	digitalWrite(MOTORDATA, LOW);		// SER_PORT &= ~_BV(SER);
	
	for (i = 0; i < 8; ++i) {
		digitalWrite(MOTORCLK, LOW);	// CLK_PORT &= ~_BV(CLK);4
		//SER_PORT |= _BV(SER) :  SER_PORT &= ~_BV(SER);
		(latch_state & _BV(7-i)) ? digitalWrite(MOTORDATA, HIGH) : digitalWrite(MOTORDATA, LOW);
		digitalWrite(MOTORCLK, HIGH);		//CLK_PORT |= _BV(CLK);
	}
	digitalWrite(MOTORLATCH, HIGH);		//LATCH_PORT |= _BV(LATCH);
}

// 
void AFMotorController::enable(void) {
	pinMode(MOTORLATCH, OUTPUT);
	pinMode(MOTORENABLE, OUTPUT);
	pinMode(MOTORDATA, OUTPUT);
	pinMode(MOTORCLK, OUTPUT);

	latch_state = 0;

	latch_tx();  // "reset"

	//ENABLE_PORT &= ~_BV(ENABLE); // enable the chip outputs!
	digitalWrite(MOTORENABLE, LOW);
}

AF_DCMotor::AF_DCMotor(uint8_t num, uint8_t freq) {
	motornum = num;
	pwmfreq = freq;

	MC.enable();

	// MOTOR2_A: 1, MOTOR1_A: 2, MOTOR1_B: 3, MOTOR2_B: 4
	latch_state &= ~_BV(MOTOR1_A) & ~_BV(MOTOR1_B); // 2, 3번 BIT를 0으로 설정
    MC.latch_tx();
    initPWM1(freq);
	latch_state &= ~_BV(MOTOR2_A) & ~_BV(MOTOR2_B); // 1, 4번 BIT를 0으로 설정
    MC.latch_tx();
    initPWM2(freq);
}

void AF_DCMotor::run(uint8_t cmd) {
	uint8_t a, b;
	switch (motornum) {
	case 1: a = MOTOR1_A; b = MOTOR1_B; break;
	case 2: a = MOTOR2_A; b = MOTOR2_B; break;
	}
  
	switch (cmd) {
	case FORWARD:
		latch_state |= _BV(a);
		latch_state &= ~_BV(b); 	// A high, B low
		MC.latch_tx();
		break;
	case BACKWARD:
		latch_state &= ~_BV(a);		// A low, B high
		latch_state |= _BV(b); 
		MC.latch_tx();
		break;
	case RELEASE:
		latch_state &= ~_BV(a);     // A and B both low
		latch_state &= ~_BV(b); 
		MC.latch_tx();
		break;
	}
}

void AF_DCMotor::setSpeed(uint8_t speed) {
	switch (motornum) {
	case 1: setPWM1(speed); break;
	case 2: setPWM2(speed); break;
	}
}

inline void initPWM1(uint8_t freq) {
    // use PWM from timer2A on PB3 (Arduino pin #11)
    TCCR2A |= _BV(COM2A1) | _BV(WGM20) | _BV(WGM21); // fast PWM, turn on oc2a
    TCCR2B = freq & 0x7;
    OCR2A = 0;
	pinMode(11, OUTPUT);
}

inline void setPWM1(uint8_t s) {
    // use PWM from timer2A on PB3 (Arduino pin #11)
    OCR2A = s;
}

inline void initPWM2(uint8_t freq) {
    // use PWM from timer2B (pin 3)
    TCCR2A |= _BV(COM2B1) | _BV(WGM20) | _BV(WGM21); // fast PWM, turn on oc2b
    TCCR2B = freq & 0x7;
    OCR2B = 0;
	pinMode(3, OUTPUT);
}

inline void setPWM2(uint8_t s) {
    // use PWM from timer2A on PB3 (Arduino pin #11)
    OCR2B = s;
}

void digitalWrite(uint8_t pin, uint8_t val) {
	uint8_t timer = digitalPinToTimer(pin);			// 해당 핀이 PWM 가능 핀일경우 대응하는 TIMER 레지스터 주소 
	uint8_t bit = digitalPinToBitMask(pin);			// 해당 핀의 포트의 대응하는 비트 자릿수
	uint8_t port = digitalPinToPort(pin);			// 핀 번호에 해당하는 포트의 데이터 레지스터(DDRx) 1-byte 읽어옴.
	volatile uint8_t *out;

	// If the pin that support PWM output, we need to turn it off
	// before doing a digital write. (NOT_ON_TIMER == 0) 
	if (timer != NOT_ON_TIMER) turnOffPWM(timer);	// PWM 가능 핀을 digitalWrite 할 때는 PWM 및 timer를 꺼야 함.

	out = portOutputRegister(port);

	uint8_t oldSREG = SREG;
	cli();

	if (val == LOW) {
		*out &= ~bit;
	} else {
		*out |= bit;
	}

	SREG = oldSREG;
}

int digitalRead(uint8_t pin) {
	uint8_t timer = digitalPinToTimer(pin);
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);

	if (port == NOT_A_PIN) return LOW;

	// If the pin that support PWM output, we need to turn it off
	// before getting a digital reading.
	if (timer != NOT_ON_TIMER) turnOffPWM(timer);

	if (*portInputRegister(port) & bit) return HIGH;
	return LOW;
}
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))					// clear bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))					// set bit
#define _MMIO_BYTE(mem_addr) (*(volatile uint8_t *)(mem_addr))
#define _MMIO_WORD(mem_addr) (*(volatile uint16_t *)(mem_addr))
#define _MMIO_DWORD(mem_addr) (*(volatile uint32_t *)(mem_addr))
static void turnOffPWM(uint8_t timer)
{
	switch (timer)
	{
		#if defined(TCCR1A) && defined(COM1A1)
		case TIMER1A:   cbi(TCCR1A, COM1A1);    break;
		#endif
		#if defined(TCCR1A) && defined(COM1B1)
		case TIMER1B:   cbi(TCCR1A, COM1B1);    break;
		#endif
		#if defined(TCCR1A) && defined(COM1C1)
		case TIMER1C:   cbi(TCCR1A, COM1C1);    break;
		#endif
		
		#if defined(TCCR2) && defined(COM21)
		case  TIMER2:   cbi(TCCR2, COM21);      break;
		#endif
		
		#if defined(TCCR0A) && defined(COM0A1)
		case  TIMER0A:  cbi(TCCR0A, COM0A1);    break;
		#endif
		
		#if defined(TCCR0A) && defined(COM0B1)
		case  TIMER0B:  cbi(TCCR0A, COM0B1);    break;
		#endif
		#if defined(TCCR2A) && defined(COM2A1)
		case  TIMER2A:  cbi(TCCR2A, COM2A1);    break;
		#endif
		#if defined(TCCR2A) && defined(COM2B1)
		case  TIMER2B:  cbi(TCCR2A, COM2B1);    break;
		#endif
		
		#if defined(TCCR3A) && defined(COM3A1)
		case  TIMER3A:  cbi(TCCR3A, COM3A1);    break;
		#endif
		#if defined(TCCR3A) && defined(COM3B1)
		case  TIMER3B:  cbi(TCCR3A, COM3B1);    break;
		#endif
		#if defined(TCCR3A) && defined(COM3C1)
		case  TIMER3C:  cbi(TCCR3A, COM3C1);    break;
		#endif

		#if defined(TCCR4A) && defined(COM4A1)
		case  TIMER4A:  cbi(TCCR4A, COM4A1);    break;
		#endif					
		#if defined(TCCR4A) && defined(COM4B1)
		case  TIMER4B:  cbi(TCCR4A, COM4B1);    break;
		#endif
		#if defined(TCCR4A) && defined(COM4C1)
		case  TIMER4C:  cbi(TCCR4A, COM4C1);    break;
		#endif			
		#if defined(TCCR4C) && defined(COM4D1)
		case TIMER4D:	cbi(TCCR4C, COM4D1);	break;
		#endif			
			
		#if defined(TCCR5A)
		case  TIMER5A:  cbi(TCCR5A, COM5A1);    break;
		case  TIMER5B:  cbi(TCCR5A, COM5B1);    break;
		case  TIMER5C:  cbi(TCCR5A, COM5C1);    break;
		#endif
	}
}