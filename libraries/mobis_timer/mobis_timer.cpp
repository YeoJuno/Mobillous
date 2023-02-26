#include "Arduino.h"
#include "mobis_io.h"
#include "mobis_timer.h"

void FrequencyTimer2::enable() {
    /* TCCR2A: Timer/Counter Control Register A
    Set TCCR2A[6](COM2A0) bit to set/toggle OC2A on compare match */
    FrequencyTimer2::enabled = true;
    _BIT_SET_(*(volatile uint8_t *)(0xB0), 6);      // TCCR2A |= _BV(COM2A0);
}
void FrequencyTimer2::disable() {
    /* TCCR2A: Timer/Counter Control Register A
    Set TCCR2A[6](COM2A0) bit to clear/disable OC2A */
    FrequencyTimer2::enabled = false;
    _BIT_UNSET_(*(volatile uint8_t *)(0xB0), 6);    // TCCR2A &= _BV(COM2A0);
}

void FrequencyTimer2::setOnOverflow(void (*func)(void)) {
    FrequencyTimer2::onOverflow = func;
    /* TIMSK2(0x70): Timer/Counter2 Interrupt Mask Register
    Set 'Timer2A Interrupt Enable' bit. */
    if (func) _BIT_SET_(*(volatile uint8_t *)(0x70), 1);
    else _BIT_UNSET_(*(volatile uint8_t *)(0x70), 1);
}

void FrequencyTimer2::setPeriod(unsigned long period) {
    uint8_t pre, top;
    if (period == 0) period = 1;
    // we work with half-cycles before the toggle (?)
    period *= (clockCyclesPerMicrosecond() / 2);
	
    if (period <= 256L)                 pre = 1, top = period - 1;
    else if (period <= (256L << 3))     pre = 2, top = (period >> 3) - 1;
    else if (period <= (256L << 5))     pre = 3, top = (period >> 5) - 1;
    else if (period <= (256L << 6))     pre = 4, top = (period >> 6) - 1;
    else if (period <= (256L << 7))     pre = 5, top = (period >> 7) - 1;
    else if (period <= (256L << 8))     pre = 6, top = (period >> 8) - 1;
    else if (period <= (256L << 10))    pre = 7, top = (period >> 10) - 1;
    else  pre = 7, top = 255;
	
    _BIT_RESET_(*(volatile uint8_t *)(0xB1));       // TCCR2B = 0;
    _BIT_RESET_(*(volatile uint8_t *)(0xB0));       // TCCR2A = 0;
    _BIT_RESET_(*(volatile uint8_t *)(0xB2));       // TCNT2 = 0;
    // When AS2 is written to zero, Timer/Counter2 is clocked from the I/O clock.
    OCR2A = top;
    _BIT_UNSET_(*(volatile uint8_t *)(0xB6), 5);    // ASSR &= ~_BV(AS2);
    _BITS_SET_(*(volatile uint8_t *)(0xB6), top);   // TCCR2B = pre;
    TCCR2A = (_BIT_(1) | (FrequencyTimer2::enabled ? _BIT_(6) : 0));
    // TCCR2A = (_BV(WGM21) | (FrequencyTimer2::enabled ? _BV(COM2A0) : 0));
}

unsigned long FrequencyTimer2::getPeriod() {
    /* TCCR2B(0xB1): Timer/Counter Control Register B
    [2:0]; CS22, CS21, CS20 bits; Clock source select bits */
    uint8_t clock_source = ((*(volatile uint8_t *)(0xB1)) & 0x07);
    /* OCR2A(0xB3): Top(Upper boundary) of Timer2A which compared with TCNT2 */
    uint8_t top = (*(volatile uint8_t *)(0xB3));

    uint8_t shift;
    switch(clock_source) {
    case 0x00: shift = 0;  break;  // No prescailing
    case 0x01: shift = 0;  break;  // No prescailing
    case 0x02: shift = 3;  break;  // T2A source divided by 8 from prescaler
    case 0x03: shift = 5;  break;  // T2A source divided by 32 from prescaler
    case 0x04: shift = 6;  break;  // T2A source divided by 64 from prescaler
    case 0x05: shift = 7;  break;  // T2A source divided by 128 from prescaler
    case 0x06: shift = 8;  break;  // T2A source divided by 256 from prescaler
    case 0x07: shift = 10; break;  // T2A source divided by 1024 from prescaler
    }
    // Get period within a microsecond.
    return (((unsigned long)(top + 1) << (shift + 1)) + 1) / clockCyclesPerMicrosecond();
}

/* ISR(TIMER2_COMPA_vect) {
    static uint8_t intHandler = 0;
    if ( !intHandler && FrequencyTimer2::onOverflow ) {
        intHandler = 1;
        (*FrequencyTimer2::onOverflow)();
        intHandler = 0;
    }
} */