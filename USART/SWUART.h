/*
 * 		Deskwizard's AVR Code Reference for software UART
 *
 *		File: SWUART.h 		->			Deskwizard's AVR Code Reference for software UART (header)
 */

#ifndef _DW_CODE_REFERENCE_SWUART
#define  _DW_CODE_REFERENCE_SWUART

// Transmitter configuration
#define SWUART_TXPORT 	PORTD		// Software UART TX Pin Port register
#define SWUART_TXDIR	 	DDRD		// Software UART TX Pin Data Direction register
#define SWUART_TX			PD6			// Software UART TX Port Pin

// Receiver configuration
#define SWUART_RXPIN 		PIND		// Software UART TX PIN register (input read register)
#define SWUART_RX 			PD2			// Software UART RX Port Pin

#define SWUART_BUFFERSIZE 32			// Software UART buffer size

// Initialization
void SWUARTInit(void);

// Transmission
void SWUARTSendChar(uint8_t _char);

// Reception
int8_t SWUARTDataReady(void);		// Call SWUARTReceive() after this check to receive the byte.
int8_t SWUARTReceive(void);			//  *** DO NOT CALL without first testing with SWUARTDataReady() as it modifies the next_read pointer ***

// Status bits
#define TX_BUSY 0					// Is set if a byte is in transmission
#define RX_BUSY 1					// Is set if a byte is being received

// TODO: Framing error handling
//#define RX_FRAME_ERR 2		// Is set if the stop byte isn't a 1

#endif
