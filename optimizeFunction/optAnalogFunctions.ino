#include "wiring_private.h"
#include "pins_arduino.h"

uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode) {
	analog_reference = mode;
}

int analogRead(uint8_t pin) {
	pin -= 14;
	ADMUX = (analog_reference << 6) | (pin & 0x07);
	sbi(ADCSRA, ADSC);	// start the conversion
	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));
	// ADC macro takes care of reading ADC register.
	return ADC;
}

void analogWrite(uint8_t pin, int val) {
	switch(digitalPinToTimer(pin)) {
	case TIMER0A:
		// connect pwm to pin on timer 0, channel A
		sbi(TCCR0A, COM0A1);
		OCR0A = val; // set pwm duty
		break;
	case TIMER0B:
		// connect pwm to pin on timer 0, channel B
		sbi(TCCR0A, COM0B1);
		OCR0B = val; // set pwm duty
		break;
	case TIMER1A:
		// connect pwm to pin on timer 1, channel A
		sbi(TCCR1A, COM1A1);
		OCR1A = val; // set pwm duty
		break;
	case TIMER1B:
		// connect pwm to pin on timer 1, channel B
		sbi(TCCR1A, COM1B1);
		OCR1B = val; // set pwm duty
		break;
	case TIMER2A:
		// connect pwm to pin on timer 2, channel A
		sbi(TCCR2A, COM2A1);
		OCR2A = val; // set pwm duty
		break;
	case TIMER2B:
		// connect pwm to pin on timer 2, channel B
		sbi(TCCR2A, COM2B1);
		OCR2B = val; // set pwm duty
		break;
	}
}
