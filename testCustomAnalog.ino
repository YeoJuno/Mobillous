#include <SoftwareSerial.h>

#define _BIT_(x)                    ( 1 << (x) )
#define _BIT_RESET_(x)              ( (x) = 0x00 )
#define _BIT_SET_(x, pos)           ( (x) |= (_BIT_(pos)) )
#define _BIT_TOGGLE_(x, pos)        ( (x) ^= (_BIT_(pos)) )
#define _BIT_UNSET_(x, pos)         ( (x) &= (~(_BIT_(pos))) )
#define _BIT_CHECK_(x, pos)         ( ((x) >> (pos)) & 0x01 )
#define _BITS_SET_(x, bits)         ( (x) |= (bits) )
#define _BITS_TOGGLE_(x, bits)      ( (x) ^= (bits) )
#define _BITS_UNSET_(x, bits)       ( (x) &= (~(bits)) )
#define _BIT_WRITE_(x, pos, data)   ( (data) ? ((_BIT_SET_((x), (pos)))) : ((_BIT_UNSET_((x), (pos)))) )

typedef struct UnoPin {
    volatile uint8_t* addrPort;    // Address of PORTx.
    volatile uint8_t* addrDdr;     // Address of DDRx.
    volatile uint8_t* addrPin;     // Address of PINx.
    uint8_t pinNumInPort;          // Pin number within the port.
} UnoPin;

static const UnoPin unoPin[] = {
    /* ------ Port D (Digital 0 ~ 7) ------- */
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 0},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 1},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 2},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 3},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 4},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 5},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 6},
    {(volatile uint8_t *)0x2B, (volatile uint8_t *)0x2A, (volatile uint8_t *)0x29, 7},
    /* ------ Port B (Digital 8 ~ 13) ------- */
    {(volatile uint8_t *)0x25, (volatile uint8_t *)0x24, (volatile uint8_t *)0x23, 0},
    {(volatile uint8_t *)0x25, (volatile uint8_t *)0x24, (volatile uint8_t *)0x23, 1},
    {(volatile uint8_t *)0x25, (volatile uint8_t *)0x24, (volatile uint8_t *)0x23, 2},
    {(volatile uint8_t *)0x25, (volatile uint8_t *)0x24, (volatile uint8_t *)0x23, 3},
    {(volatile uint8_t *)0x25, (volatile uint8_t *)0x24, (volatile uint8_t *)0x23, 4},
    {(volatile uint8_t *)0x25, (volatile uint8_t *)0x24, (volatile uint8_t *)0x23, 5},
    /* ------ Port C (Analog 0 ~ 5) ------- */
    {(volatile uint8_t *)0x28, (volatile uint8_t *)0x27, (volatile uint8_t *)0x26, 0},
    {(volatile uint8_t *)0x28, (volatile uint8_t *)0x27, (volatile uint8_t *)0x26, 1},
    {(volatile uint8_t *)0x28, (volatile uint8_t *)0x27, (volatile uint8_t *)0x26, 2},
    {(volatile uint8_t *)0x28, (volatile uint8_t *)0x27, (volatile uint8_t *)0x26, 3},
    {(volatile uint8_t *)0x28, (volatile uint8_t *)0x27, (volatile uint8_t *)0x26, 4},
    {(volatile uint8_t *)0x28, (volatile uint8_t *)0x27, (volatile uint8_t *)0x26, 5}
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
inline uint16_t testAnalogRead(uint8_t pin) {
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

uint16_t value = 0;

void measureSpeedOfCode() {
    TCCR1A = 0;             // Set Timer/Counter Control Register A to 0 for initialization.
    TCCR1B = bit(CS10);     // Set 11st bit to 1 of Timer/Counter Control Register B.
    TCNT1 = 0;              // Initialize register which indicates Timer1's value(or counts).
    unsigned int cycles;

    /* Code under test */
    // do something
    value = testAnalogRead(15);
    //value = analogRead(15);
    /* End of code under test */

    cycles = TCNT1;
    Serial.print("CPU Cycles: "); Serial.print(cycles - 1);
    Serial.print(", "); Serial.print((float)(cycles - 1) / 16); Serial.println("ms");
}

void setup() {
    Serial.begin(115200);
    Serial.println("------Test starts!--------");
    // volatile uint8_t data = (*(volatile uint8_t *)(0x7A));
    // Serial.print("ADSCRA: ");
    // Serial.println(data);
    directSetPinIn(15);             // pinMode(A1, INPUT)
    directSetPinOut(13);            // pinMode(LED_BUILTIN, OUTPUT);
    _BITS_UNSET_((*(volatile uint8_t *)(0x7A)), 0x03);

    for (uint8_t i = 0; i < 5; ++i) {
        measureSpeedOfCode();        
        Serial.print("Sensor value = ");
        Serial.println(value);
        delay(100);
    }
}

void loop() {
    
}