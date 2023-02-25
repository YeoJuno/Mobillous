static uint8_t pinMask[] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, // PORT D pins (8-pins)
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20,             // PORT B pins (6-pins)
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20              // PORT C pins (6-pins)
};
inline void directSetPinOut(uint8_t pin) {
    // Get DDRx value of Portx with pin is included.
    volatile uint8_t *reg_ddr = portModeRegister(digitalPinToPort(pin));
    *reg_ddr |= pinMask[pin];
}
inline void directSetPinIn(uint8_t pin) {
    // Get DDRx and PORTx value of Portx with pin is included.
    volatile uint8_t *reg_ddr = portModeRegister(digitalPinToPort(pin));
    volatile uint8_t *reg_pOut = portOutputRegister(digitalPinToPort(pin));
    *reg_ddr &= ~pinMask[pin];
    *reg_pOut &= ~pinMask[pin];
}
/* [!]. Of course there is no 'atomicity' in this code.
    There is a trace off between safety and performance. */
inline bool directDigitalRead(uint8_t pin) {
    // Get PINx value of Portx with pin is included.
    // [Q]. Is it necessary to turn off PWM timer with pins which supports PWM?
    volatile uint8_t *reg_pIn = portInputRegister(digitalPinToPort(pin));
    return (*reg_pIn & pinMask[pin]);
}
inline void directDigitalWrite(uint8_t pin, bool data) {
    volatile uint8_t *reg_pOut = portOutputRegister(digitalPinToPort(pin));
    // [Q]. Is it necessary to turn off PWM timer with pins which supports PWM?
    (data) ? (*reg_pOut |= pinMask[pin]) : (*reg_pOut &= ~pinMask[pin]);
}
inline void directDigitalToggle(uint8_t pin) {
    volatile uint8_t *reg_pOut = portOutputRegister(digitalPinToPort(pin));
    *reg_pOut ^= pinMask[pin];
}

void setup() {
    directSetPinOut(13);
}

void loop() {
    directDigitalToggle(13);
    delay(1000);
}
