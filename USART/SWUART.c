/*
 * 		Deskwizard's AVR Code Reference for software UART
 *
 *		File: SWUART.c 		->			Deskwizard's AVR Code Reference for software UART (source)
 */

#include <avr/io.h>							// Include AVR IO definitions
#include <avr/interrupt.h>				// Include AVR Interrupts definitions
#include "SWUART.h"						// Include Software UART Reference Code definitions

// Software UART registers
static uint8_t SWUART_Status;				// UART status
static uint8_t SWUART_RXByte;				// Byte being received
static uint8_t SWUART_TXByte;				// Byte being transmitted
static uint8_t UART_TXTickCount;			// Tick counter for transmission
static uint8_t UART_RXTickCount;			// Tick counter for the reception
static uint8_t SWUART_TXBitCount;		// Bit counter for transmission
static uint8_t SWUART_RXBitCount;		// Bit count for reception
static uint8_t next_write_2;					// Pointer to buffer (transmission)
static uint8_t next_read_2;					// Pointer to buffer (reception)

static unsigned char SWUART_Buffer[SWUART_BUFFERSIZE]; // Circular buffer for reception


// ********************************** Initialization  **********************************/

// Configures Timer1 Mode, Prescaler and Output Compare A Interrupt,
// initialize the variables and set TX pin mode and state
void SWUARTInit(void) {

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
	SWUART_Status = 0;

	// Set TX pin as output/high
	SWUART_TXDIR |= (1 << SWUART_TX);
	SWUART_TXPORT |= (1 << SWUART_TX);

	// Enable global interrupts
	sei();
}

// ********************************** Transmission  **********************************/

// Send the character to the software UART, waiting until the output buffer is available
void SWUARTSendChar(uint8_t byteToSend) {

	while ((SWUART_Status & (1 << TX_BUSY)) == 1)
		;		// wait...
	{
		// Once the system isn't busy, load the register
		SWUART_TXByte = byteToSend;

		// And tell it there's something to send
		SWUART_Status |= (1 << TX_BUSY);
	}
} // void SWUARTSendChar(int8_t _char)


// ********************************** Reception  **********************************/

// Returns 1 if there's a byte in the software UART buffer, 0 otherwise
// Call SWUARTReceive() after this check to receive the byte.
int8_t SWUARTDataReady(void) {

	if (next_read_2 != next_write_2) {
		return 1;
	}
	else {
		return 0;
	}
} // int8_t SWUARTDataReady(void)


// Returns a character from the soft UART
//  **** Do *NOT* call without first testing with SWUARTDataReady() as it modifies the next_read pointer ***
int8_t SWUARTReceive(void) {
	int8_t receivedByte;
	receivedByte = SWUART_Buffer[next_read_2++];
	if (next_read_2 == SWUART_BUFFERSIZE)
		next_read_2 = 0;
	return receivedByte;
} // int8_t SWUARTReceive(void)

// ********************************** Interrupt  **********************************/

ISR (TIMER1_COMPA_vect) {

	// 	  ->	Triggered 4 * baudrate
	// 				1) Send data (if required)
	//				2) Receive incoming data (if available)

	if (SWUART_Status & (1 << TX_BUSY)) {

		// Increment the tick - we only do things every four ticks
		UART_TXTickCount++;

		if (UART_TXTickCount == 4) {	// Start transmission
			UART_TXTickCount = 0;

			if (SWUART_TXBitCount == 0) {	// Start bit
				SWUART_TXPORT &= ~((1 << SWUART_TX));	// Clear start bit output
				SWUART_TXBitCount++;
			} else { // No
				if (SWUART_TXBitCount != 9) {
					// Data bits
					if (SWUART_TXByte & 1)				// Low bit set?
						SWUART_TXPORT |= (1 << SWUART_TX);	// Set data stream bit
					else
						SWUART_TXPORT &= ~((1 << SWUART_TX));	// Clear data stream bit
					SWUART_TXBitCount++;			// Increment bit count
					SWUART_TXByte /= 2;				// Shift data right
				} else {
					// Stop bit
					SWUART_TXPORT |= (1 << SWUART_TX);
					SWUART_TXBitCount++;
				}
			}
			// If TX bit is > than 9, we're done
			if (SWUART_TXBitCount > 9) {
				SWUART_TXBitCount = 0;							// Clear bit counter
				SWUART_Status &= ~((1 << TX_BUSY)); 	// Clear busy status
			}
		}
	}

	// We may be receiving something if we're *not* yet receiving,
	// we check every clock tick to see if the input line has gone into a stop bit.
	// If it did, wait for half a bit period and then sample every four ticks
	// to put together the RX data

	uint8_t uart_bitpattern[9] = { 0, 1, 2, 4, 8, 16, 32, 64, 128 };

	if ((SWUART_Status & (1 << RX_BUSY)) == 0) {		// If idle

		if ((SWUART_RXPIN & (1 << SWUART_RX)) == 0) {		// Start bit?

			// Set the tick count to 2, so we get the sample near the middle of the bit
			UART_RXTickCount = 2;

			SWUART_Status |= (1 << RX_BUSY);					// Set busy flag
			SWUART_RXBitCount = 0;
		}
	} else {		// Receiving

		UART_RXTickCount++;		// Increase tick count

		if (UART_RXTickCount == 4) {	// We only sample when UART_RXTickCount == 0
			UART_RXTickCount = 0;

			if (SWUART_RXBitCount == 0) {	// Start bit ?
				if ((SWUART_RXPIN & (1 << SWUART_RX)) == 0) { // Start bit
					SWUART_RXBitCount++;
					SWUART_RXByte = 0;
				} else {	// Invalid start bit, reset state
					SWUART_Status |= ~((1 << RX_BUSY));
				}
			} else {
				if (SWUART_RXBitCount < 9) {	// Data bit
					if (SWUART_RXPIN & (1 << SWUART_RX)) { // If data bit == 1, add bit value to SWUART_RXByte
						SWUART_RXByte += uart_bitpattern[SWUART_RXBitCount];
					}
					SWUART_RXBitCount++;
				} else {
					if (SWUART_RXBitCount == 9) { // Stop bit

						// TODO: Framing error handling
						// We're going to assume it's a valid bit, though we could check for framing
						// error here, and simply use this bit to wait for the first stop bit period

						SWUART_RXBitCount++;
					} else {	// Received.
						// Reset state
						SWUART_Status &= ~((1 << RX_BUSY));
						// Store data in buffer
						SWUART_Buffer[next_write_2++] = SWUART_RXByte;
						if (next_write_2 == SWUART_BUFFERSIZE)
							next_write_2 = 0;
					}
				}
			}
		}
	}
} // ISR (TIMER1_COMPA_vect)
