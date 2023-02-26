#ifndef _MOBIS_IO_H_
#define _MOBIS_IO_H_

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
    {(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 0}, 
	{(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 1}, 
	{(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 2},
    {(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 3}, 
	{(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 4}, 
	{(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 5},
    {(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 6}, 
	{(volatile uint8_t*)0x2B, (volatile uint8_t*)0x2A, (volatile uint8_t*)0x29, 7},
    /* ------ Port B (Digital 8 ~ 13) ------- */
    {(volatile uint8_t*)0x25, (volatile uint8_t*)0x24, (volatile uint8_t*)0x23, 0}, 
	{(volatile uint8_t*)0x25, (volatile uint8_t*)0x24, (volatile uint8_t*)0x23, 1}, 
	{(volatile uint8_t*)0x25, (volatile uint8_t*)0x24, (volatile uint8_t*)0x23, 2},
    {(volatile uint8_t*)0x25, (volatile uint8_t*)0x24, (volatile uint8_t*)0x23, 3}, 
	{(volatile uint8_t*)0x25, (volatile uint8_t*)0x24, (volatile uint8_t*)0x23, 4}, 
	{(volatile uint8_t*)0x25, (volatile uint8_t*)0x24, (volatile uint8_t*)0x23, 5},
    /* ------ Port C (Analog 0 ~ 5) ------- */
    {(volatile uint8_t*)0x28, (volatile uint8_t*)0x27, (volatile uint8_t*)0x26, 0}, 
	{(volatile uint8_t*)0x28, (volatile uint8_t*)0x27, (volatile uint8_t*)0x26, 1}, 
	{(volatile uint8_t*)0x28, (volatile uint8_t*)0x27, (volatile uint8_t*)0x26, 2},
    {(volatile uint8_t*)0x28, (volatile uint8_t*)0x27, (volatile uint8_t*)0x26, 3}, 
	{(volatile uint8_t*)0x28, (volatile uint8_t*)0x27, (volatile uint8_t*)0x26, 4}, 
	{(volatile uint8_t*)0x28, (volatile uint8_t*)0x27, (volatile uint8_t*)0x26, 5}
};

/* Substitute of 'pinMode(, OUTPUT)'. Modify DDRx register. */
void directSetPinOut(uint8_t pin);
/* Substitute of 'pinMode(, INPUT)'. Modify DDRx, PINx register. */
void directSetPinIn(uint8_t pin);
/* Substitute of 'digitalRead()'. Modify PINx register. */
uint8_t directDigitalRead(uint8_t pin);
/* Substitute of 'digitalWrite()'. Modify PORTx register. */
void directDigitalWrite(uint8_t pin, uint8_t data) ;
void directDigitalToggle(uint8_t pin);
uint16_t directAnalogRead(uint8_t pin);

#endif