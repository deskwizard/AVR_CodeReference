/*
 * 		Deskwizard's AVR Code Reference for software USART
 *
 *		File: SWUSART.c 		->			Deskwizard's AVR Code Reference for software USART (source)
 *
 *		Based on http://www.avrfreaks.net/projects/full-duplex-software-serial-port
 */

#include <avr/io.h>							// Include AVR IO definitions
#include <avr/interrupt.h>				// Include AVR Interrupts definitions
#include "SWUSART.h"						// Include Software USART Reference Code definitions

// Software USART registers
static uint8_t SWUSART_Status;				// USART status
static uint8_t SWUSART_RXByte;				// Byte being received
static uint8_t SWUSART_TXByte;				// Byte being transmitted
static uint8_t SWUSART_TXTickCount;			// Tick counter for transmission
static uint8_t SWUSART_RXTickCount;			// Tick counter for the reception
static uint8_t SWUSART_TXBitCount;		// Bit counter for transmission
static uint8_t SWUSART_RXBitCount;		// Bit count for reception
static uint8_t next_write_2;					// Pointer to buffer (transmission)
static uint8_t next_read_2;					// Pointer to buffer (reception)

static unsigned char SWUSART_Buffer[SWUSART_BUFFERSIZE]; // Circular buffer for reception


// ********************************** Initialization  **********************************/

// Configures Timer1 Mode, Prescaler and Output Compare A Interrupt,
// initialize the variables and set TX pin mode and state
void SWUSARTInit(void) {

	// Disable global interrupts while configuring timer
	cli();

	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS10);		// CTC mode with OCR1A top, clock/1 prescaler

	OCR1A = 0xCD;// preset compare counter to 208 (19200bps @ 16mhz) 	* TESTED > 1 ms fine*
	//OCR1A = 0x1A0;			// preset compare counter to 416 (9600bps @ 16mhz) 		* TESTED > 2 ms MIN.*
	//OCR1A = 0x341;			// preset compare counter to 833 (4800bps @ 16mhz) 		* TESTED > 3 ms MIN.*
	//OCR1A = 0x682;			// preset compare counter to 1666 (2400bps @ 16mhz) 		* TESTED > 5 ms MIN.*
	//OCR1A = 0xD04;			// preset compare counter to 3332 (1200bps @ 16mhz) 		* TESTED > 10 ms MIN.*

	// Enable Output Compare A on Timer1
	TIMSK |= (1 << OCIE1A);

	// Initialize the buffer pointers
	next_read_2 = 0;
	next_write_2 = 0;

	// Initialize the status variable
	SWUSART_Status = 0;

	// Set TX pin as output/high
	SWUSART_TXDIR |= (1 << SWUSART_TX);
	SWUSART_TXPORT |= (1 << SWUSART_TX);

	// Enable global interrupts
	sei();
}

// ********************************** Transmission  **********************************/

// Send the character to the software USART, waiting until the output buffer is available
void SWUSARTSendChar(int8_t byteToSend) {

	while ((SWUSART_Status & (1 << TX_BUSY)) == 1)
		;		// wait...
	{
		// Once the system isn't busy, load the register
		SWUSART_TXByte = byteToSend;

		// And tell it there's something to send
		SWUSART_Status |= (1 << TX_BUSY);
	}
} // void SWUSARTSendChar(int8_t _char)


// ********************************** Reception  **********************************/

// Returns 1 if there's a byte in the software USART buffer, 0 otherwise
// Call SWUSARTReceive() after this check to receive the byte.
int8_t SWUSARTDataReady(void) {

	if (next_read_2 != next_write_2) {
		return 1;
	}
	else {
		return 0;
	}
} // int8_t SWUSARTDataReady(void)


// Returns a character from the soft USART
//  **** Do *NOT* call without first testing with SWUSARTDataReady() as it modifies the next_read pointer ***
int8_t SWUSARTReceive(void) {
	int8_t receivedByte;
	receivedByte = SWUSART_Buffer[next_read_2++];
	if (next_read_2 == SWUSART_BUFFERSIZE)
		next_read_2 = 0;
	return receivedByte;
} // int8_t SWUSARTReceive(void)

// ********************************** Interrupt  **********************************/

ISR (TIMER1_COMPA_vect) {

	// 	  ->	Triggered 4 * baudrate
	// 				1) Send data (if required)
	//				2) Receive incoming data (if available)

	if (SWUSART_Status & (1 << TX_BUSY)) {

		// Increment the tick - we only do things every four ticks
		SWUSART_TXTickCount++;

		if (SWUSART_TXTickCount == 4) {	// Start transmission
			SWUSART_TXTickCount = 0;

			if (SWUSART_TXBitCount == 0) {	// Start bit
				SWUSART_TXPORT &= ~((1 << SWUSART_TX));	// Clear start bit output
				SWUSART_TXBitCount++;
			} else { // No
				if (SWUSART_TXBitCount != 9) {
					// Data bits
					if (SWUSART_TXByte & 1)				// Low bit set?
						SWUSART_TXPORT |= (1 << SWUSART_TX);	// Set data stream bit
					else
						SWUSART_TXPORT &= ~((1 << SWUSART_TX));	// Clear data stream bit
					SWUSART_TXBitCount++;			// Increment bit count
					SWUSART_TXByte /= 2;				// Shift data right
				} else {
					// Stop bit
					SWUSART_TXPORT |= (1 << SWUSART_TX);
					SWUSART_TXBitCount++;
				}
			}
			// If TX bit is > than 9, we're done
			if (SWUSART_TXBitCount > 9) {
				SWUSART_TXBitCount = 0;							// Clear bit counter
				SWUSART_Status &= ~((1 << TX_BUSY)); 	// Clear busy status
			}
		}
	}

	// We may be receiving something if we're *not* yet receiving,
	// we check every clock tick to see if the input line has gone into a stop bit.
	// If it did, wait for half a bit period and then sample every four ticks
	// to put together the RX data

	uint8_t uart_bitpattern[9] = { 0, 1, 2, 4, 8, 16, 32, 64, 128 };

	if ((SWUSART_Status & (1 << RX_BUSY)) == 0) {		// If idle

		if ((SWUSART_RXPIN & (1 << SWUSART_RX)) == 0) {		// Start bit?

			// Set the tick count to 2, so we get the sample near the middle of the bit
			SWUSART_RXTickCount = 2;

			SWUSART_Status |= (1 << RX_BUSY);					// Set busy flag
			SWUSART_RXBitCount = 0;
		}
	} else {		// Receiving

		SWUSART_RXTickCount++;		// Increase tick count

		if (SWUSART_RXTickCount == 4) {	// We only sample when SWUSART_RXTickCount == 0
			SWUSART_RXTickCount = 0;

			if (SWUSART_RXBitCount == 0) {	// Start bit ?
				if ((SWUSART_RXPIN & (1 << SWUSART_RX)) == 0) { // Start bit
					SWUSART_RXBitCount++;
					SWUSART_RXByte = 0;
				} else {	// Invalid start bit, reset state
					SWUSART_Status |= ~((1 << RX_BUSY));
				}
			} else {
				if (SWUSART_RXBitCount < 9) {	// Data bit
					if (SWUSART_RXPIN & (1 << SWUSART_RX)) { // If data bit == 1, add bit value to SWUSART_RXByte
						SWUSART_RXByte += uart_bitpattern[SWUSART_RXBitCount];
					}
					SWUSART_RXBitCount++;
				} else {
					if (SWUSART_RXBitCount == 9) { // Stop bit

						// TODO: Framing error handling
						// We're going to assume it's a valid bit, though we could check for framing
						// error here, and simply use this bit to wait for the first stop bit period

						SWUSART_RXBitCount++;
					} else {	// Received.
						// Reset state
						SWUSART_Status &= ~((1 << RX_BUSY));
						// Store data in buffer
						SWUSART_Buffer[next_write_2++] = SWUSART_RXByte;
						if (next_write_2 == SWUSART_BUFFERSIZE)
							next_write_2 = 0;
					}
				}
			}
		}
	}
} // ISR (TIMER1_COMPA_vect)
