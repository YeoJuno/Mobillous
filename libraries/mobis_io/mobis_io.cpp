#include "Arduino.h"
#include "mobis_io.h"

/* Substitute of 'pinMode(, OUTPUT)'. Modify DDRx register. */
void directSetPinOut(uint8_t pin) {
    _BIT_SET_(*(unoPin[pin].addrDdr), unoPin[pin].pinNumInPort);
}
/* Substitute of 'pinMode(, INPUT)'. Modify DDRx, PINx register. */
void directSetPinIn(uint8_t pin) {
    _BIT_UNSET_(*(unoPin[pin].addrDdr), unoPin[pin].pinNumInPort);
    _BIT_UNSET_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}
void directSetPinInPullup(uint8_t pin) {
	_BIT_UNSET_(*(unoPin[pin].addrDdr), unoPin[pin].pinNumInPort);
	_BIT_SET_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}
/* Substitute of 'digitalRead()'. Modify PINx register. */
uint8_t directDigitalRead(uint8_t pin) {
    return _BIT_CHECK_(*(unoPin[pin].addrPin), unoPin[pin].pinNumInPort);
}
/* Substitute of 'digitalWrite()'. Modify PORTx register. */
void directDigitalWrite(uint8_t pin, uint8_t data) {
    _BIT_WRITE_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort, data);
}
void directDigitalToggle(uint8_t pin) {
    _BIT_TOGGLE_(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}
uint16_t testAnalogRead(uint8_t pin) {
    /*  ADMUX(0x7C) : select MUX[2:0] from pin number
        ADCSRA(0x7A, ADC Control and Status Register A)
            : Set ADSC(Start Conversion) to start ADC
            : ADSC goes to 0 when the conversion is complete  */
    //_BITS_SET_(*(volatile uint8_t *)(0x7C), (_BIT_(6) | ((pin - 14) & 0x07)));
    _BITS_SET_(*(volatile uint8_t *)(0x7C), ((pin - 14) & 0x07));
    _BIT_SET_((*(volatile uint8_t *)(0x7A)), 6);
	while (_BIT_CHECK_((*(volatile uint8_t *)(0x7A)), 6));
    return (*(volatile uint16_t *)(0x78));  // return ADCH(LSB 2-bit) + ADCL(8-bit) = 10-bit ADC value
}