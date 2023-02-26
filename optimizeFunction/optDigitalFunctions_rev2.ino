typedef struct UnoPin {
    volatile uint8_t* addrPort;    // Address of PORTx.
    volatile uint8_t* addrDdr;     // Address of DDRx.
    volatile uint8_t* addrPin;     // Address of PINx.
    uint8_t pinNumInPort;          // Pin number within the port.
} UnoPin;

const static UnoPin const unoPin[] = {
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

    /*
    {&PORTD, &DDRD, &PIND, 0}, {&PORTD, &DDRD, &PIND, 1}, {&PORTD, &DDRD, &PIND, 2},
    {&PORTD, &DDRD, &PIND, 3}, {&PORTD, &DDRD, &PIND, 4}, {&PORTD, &DDRD, &PIND, 5},
    {&PORTD, &DDRD, &PIND, 6}, {&PORTD, &DDRD, &PIND, 7},
    {&PORTB, &DDRB, &PINB, 0}, {&PORTB, &DDRB, &PINB, 1}, {&PORTB, &DDRB, &PINB, 2},
    {&PORTB, &DDRB, &PINB, 3}, {&PORTB, &DDRB, &PINB, 4}, {&PORTB, &DDRB, &PINB, 5},
    {&PORTC, &DDRC, &PINC, 0}, {&PORTC, &DDRC, &PINC, 1}, {&PORTC, &DDRC, &PINC, 2},
    {&PORTC, &DDRC, &PINC, 3}, {&PORTC, &DDRC, &PINC, 4}, {&PORTC, &DDRC, &PINC, 5}
    */
};

// Every below inline functions are processed within 2-cycle of CPU (which is about 0.12ms.)
inline void directSetPinOut(uint8_t pin) {
    bitSet(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}
inline void directSetPinIn(uint8_t pin) {
    bitClear(*(unoPin[pin].addrDdr), unoPin[pin].pinNumInPort);
    bitClear(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}
inline uint8_t directDigitalRead(uint8_t pin) {
    return (bitRead(*(unoPin[pin].addrPin), unoPin[pin].pinNumInPort));
}
inline void directDigitalWrite(uint8_t pin, uint8_t data) {
    bitWrite(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort, data);
}
inline void directDigitalToggle(uint8_t pin) {
    bitToggle(*(unoPin[pin].addrPort), unoPin[pin].pinNumInPort);
}

void setup() {
    
} 
 
void loop() {
    
}
