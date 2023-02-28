#include "Arduino.h"
#include "mobis_io.h"
#include "mobis_dotmatrix.h"

mobis_dot::mobis_dot(uint8_t dataPin, uint8_t clkPin, uint8_t csPin) {
    pin_MOSI	= dataPin;
    pin_CLK		= clkPin;
    pin_CS		= csPin;
    pin_MOSI	= dataPin;
	
	directSetPinOut(pin_MOSI);
	directSetPinOut(pin_CLK);
	directSetPinOut(pin_CS);
    directDigitalWrite(pin_CS, HIGH);

	uint8_t i;
    for (i = 0; i < 64; ++i)  status[i] = 0x00;
    for (i = 0; i < deviceNum; ++i) {
        xfer(i, OP_DISPLAYTEST, 0);
        setScanLimit(i, 7);
        xfer(i, OP_DECODEMODE, 0);
        clearDisplay(i);
        shutdown(i, true);
    }
}
void mobis_dot::clearDisplay(uint8_t addr) {
    uint8_t offset = addr << 3;
    for (uint8_t i = 0; i < 8; ++i) {
        status[offset + i] = 0x00;
        xfer(addr, i + 1, status[offset + i]);
    }
}
void mobis_dot::setLed(uint8_t addr, uint8_t row, uint8_t col, uint8_t state) {
    uint8_t offset = addr << 3;
    if (state) _BIT_SET_(status[offset + row], 7 - col);
    else _BIT_UNSET_(status[offset + row], 7 - col);
    xfer(addr, row + 1, status[offset + row]);
}
void mobis_dot::setRow(uint8_t addr, uint8_t row, uint8_t value)
    uint8_t offset = addr << 3;
    status[offset + row] = value;
    xfer(addr, row + 1, status[offset + row]);
}
void mobis_dot::setColumn(uint8_t addr, uint8_t col, uint8_t value) {
    uint8_t state;
    for (uint8_t row = 0; row < 8; ++row) {
        state = ((value >> (7 - row)) & 0x01);
        setLed(addr, row, col, state);
    }
}
void mobis_dot::xfer(uint8_t addr, volatile uint8_t opcode, volatile uint8_t data) {
    uint8_t offset = addr << 1;
    for (uint8_t i = 0; i < maxbytes; ++i) spidata[i] = 0x00;
    spidata[offset] = data;
    spidata[offset + 1] = opcode;
    directDigitalWrite(pin_CS, LOW);
    for (uint8_t i = 1; i >= 0; --i)
        shiftOut(pin_MOSI, pin_CLK, MSBFIRST, spidata[i]);
    directDigitalWrite(pin_CS, HIGH);
}
