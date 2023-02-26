#define _BIT_(x)                    ( 1 << (x) )
#define _BIT_RESET_(x)              ( (x) = 0x00 )
#define _BITS_EQUAL_(x, bits)       ( (x) = bits )
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

/* MAX7219's register address map */
#define OP_NOOP             0
#define OP_DIGIT0           1
#define OP_DIGIT1           2
#define OP_DIGIT2           3
#define OP_DIGIT3           4
#define OP_DIGIT4           5
#define OP_DIGIT5           6
#define OP_DIGIT6           7
#define OP_DIGIT7           8
#define OP_DECODEMODE       9
#define OP_INTENSITY        10
#define OP_SCANLIMIT        11
#define OP_SHUTDOWN         12
#define OP_DISPLAYTEST      15

class LedControl {
	uint8_t spidata[16];
	uint8_t status[64];
	uint8_t pin_CS;		// active low
	uint8_t pin_CLK;
	uint8_t pin_MOSI;
	uint8_t deviceNum;
	void spiXfer(uint8_t addr, volatile uint8_t opcode, volatile uint8_t data);

public:
	LedControl(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices = 1);
	void shutdown(uint8_t addr, uint8_t status) {
        if (status) spiXfer(addr, OP_SHUTDOWN, LOW);
        else spiXfer(addr, OP_SHUTDOWN, HIGH);
    }
	void setScanLimit(uint8_t addr, uint8_t limit) {
        spiXfer(addr, OP_INTENSITY, limit);
    }
	void setIntensity(uint8_t addr, uint8_t intensity) {
        spiXfer(addr, OP_INTENSITY, intensity);
    }
	void clearDisplay(uint8_t addr);
	void setLed(uint8_t addr, uint8_t row, uint8_t col, uint8_t state);
	void setRow(uint8_t addr, uint8_t row, uint8_t value);
	void setColumn(uint8_t addr, uint8_t col, uint8_t value);
};

LedControl::LedControl(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices) {
    pin_MOSI	= dataPin;
    pin_CLK		= clkPin;
    pin_CS		= csPin;
    pin_MOSI	= dataPin;
    deviceNum	= numDevices;
	
	directSetPinOut(pin_MOSI);
	directSetPinOut(pin_CLK);
	directSetPinOut(pin_CS);
    directDigitalWrite(pin_CS, HIGH);

	uint8_t i;
    for (i = 0; i < 64; ++i)  status[i] = 0x00;
    for (i = 0; i < deviceNum; ++i) {
        spiXfer(i, OP_DISPLAYTEST, 0);  // scanlimit is set to max on startup
        setScanLimit(i, 7);                 // decode is done in source
        spiXfer(i, OP_DECODEMODE, 0);
        clearDisplay(i);                    // we go into shutdown-mode on startup
        shutdown(i, true);
    }
}
void LedControl::clearDisplay(uint8_t addr) {
    uint8_t offset = addr << 3;
    for (uint8_t i = 0; i < 8; ++i) {
        status[offset + i] = 0x00;
        spiXfer(addr, i + 1, status[offset + i]);
    }
}
void LedControl::setLed(uint8_t addr, uint8_t row, uint8_t col, uint8_t state) {
    // if (row < 0 || row > 7 || col < 0 || col > 7) return;
    uint8_t offset = addr << 3;
    if (state) _BIT_SET_(status[offset + row], 7 - col);
    else _BIT_UNSET_(status[offset + row], 7 - col);
    spiXfer(addr, row + 1, status[offset + row]);
}
void LedControl::setRow(uint8_t addr, uint8_t row, uint8_t value) {
    // if (row < 0 || row > 7) return;
    uint8_t offset = addr << 3;
    status[offset + row] = value;
    spiXfer(addr, row + 1, status[offset + row]);
}
void LedControl::setColumn(uint8_t addr, uint8_t col, uint8_t value) {
    // if (col < 0 || col > 7) return;
    uint8_t state;
    for (uint8_t row = 0; row < 8; ++row) {
        state = ((value >> (7 - row)) & 0x01);
        setLed(addr, row, col, state);
    }
}
void LedControl::spiXfer(uint8_t addr, volatile uint8_t opcode, volatile uint8_t data) {
    uint8_t offset = addr << 1;
    uint8_t maxbytes = deviceNum * 2;
    for (uint8_t i = 0; i < maxbytes; ++i) spidata[i] = 0x00;
    spidata[offset] = data;
    spidata[offset + 1] = opcode;
    directDigitalWrite(pin_CS, LOW);
    for (uint8_t i = maxbytes; i > 0; --i)
        // shiftOut() is defined in 'wiring_shift.c'
        shiftOut(pin_MOSI, pin_CLK, MSBFIRST, spidata[i - 1]);
    directDigitalWrite(pin_CS, HIGH);
}

//#include "LedControl.h"

LedControl lc = LedControl(12, 10, 11, 1);

uint8_t hart[8] = {
    B00100100,
    B01011010,
    B10000001,
    B10000001,
    B10000001,
    B01000010,
    B00100100,
    B00011000
};
void setup() {
    lc.shutdown(0, false);
    lc.setIntensity(0, 4);
    lc.clearDisplay(0);
}
void loop() {
    for (uint8_t i = 0; i < 8; ++i) lc.setRow(0, i, hart[i]);
    delay(1000);
}