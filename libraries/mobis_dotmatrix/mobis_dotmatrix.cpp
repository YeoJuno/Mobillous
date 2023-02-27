#include "Arduino.h"
#include "mobis_io.h"
#include "mobis_dotmatrix.h"

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
        spiXfer(i, OP_DISPLAYTEST, 0);
        setScanLimit(i, 7);
        spiXfer(i, OP_DECODEMODE, 0);
        clearDisplay(i);
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