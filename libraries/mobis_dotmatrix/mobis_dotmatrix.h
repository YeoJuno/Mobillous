#ifndef _MOBIS_DOTMATRIX_H_
#define _MOBIS_DOTMATRIX_H_

/* MAX7219's register address map */
#define OP_NOOP             0x00
#define OP_DIGIT0           0x01
#define OP_DIGIT1           0x02
#define OP_DIGIT2           0x03
#define OP_DIGIT3           0x04
#define OP_DIGIT4           0x05
#define OP_DIGIT5           0x06
#define OP_DIGIT6           0x07
#define OP_DIGIT7           0x08
#define OP_DECODEMODE       0x09
#define OP_INTENSITY        0x0A
#define OP_SCANLIMIT        0x0B
#define OP_SHUTDOWN         0x0C
#define OP_DISPLAYTEST      0x0F

class mobis_dot {
	uint8_t pin_CS;			// active low
	uint8_t pin_CLK;
	uint8_t pin_MOSI;
	uint8_t status[64];
	uint8_t spidata[16];
	void xfer(uint8_t addr, volatile uint8_t opcode, volatile uint8_t data);

public:
    mobis_dot(uint8_t dataPin, uint8_t clkPin, uint8_t csPin);
	void clearDisplay(uint8_t addr);
	void setLed(uint8_t addr, uint8_t row, uint8_t col, uint8_t state);
	void setRow(uint8_t addr, uint8_t row, uint8_t value);
	void setColumn(uint8_t addr, uint8_t col, uint8_t value);
    }
	void setIntensity(uint8_t addr, uint8_t intensity) { xfer(addr, OP_INTENSITY, intensity); }
	void setScanLimit(uint8_t addr, uint8_t limit) { xfer(addr, OP_INTENSITY, limit); }
	void shutdown(uint8_t addr, uint8_t status) {
        if (status) spiXfer(addr, OP_SHUTDOWN, LOW);
        else xfer(addr, OP_SHUTDOWN, HIGH);
};
#endif
