/*
 * 		Deskwizard's AVR Code Reference for software USART
 *
 *		File: SWUSART.h 		->			Deskwizard's AVR Code Reference for software USART (header)
 *
  *		Based on http://www.avrfreaks.net/projects/full-duplex-software-serial-port
 */

#ifndef _DW_CODE_REFERENCE_SWUSART
#define  _DW_CODE_REFERENCE_SWUSART

// Transmitter configuration
#define SWUSART_TXPORT 	PORTD		// Software UART TX Pin Port register
#define SWUSART_TXDIR	 	DDRD		// Software UART TX Pin Data Direction register
#define SWUSART_TX			PD6			// Software UART TX Port Pin

// Receiver configuration
#define SWUSART_RXPIN 		PIND		// Software UART TX PIN register (input read register)
#define SWUSART_RX 			PD2			// Software UART RX Port Pin

#define SWUSART_BUFFERSIZE 32			// Software UART buffer size

// Initialization
void SWUSARTInit(void);

// Transmission
void SWUSARTSendChar(int8_t byteToSend);

// Reception
int8_t SWUSARTDataReady(void);		// Call SWUARTReceive() after this check to receive the byte.
int8_t SWUSARTReceive(void);			//  *** DO NOT CALL without first testing with SWUARTDataReady() as it modifies the next_read pointer ***

// Status bits
#define TX_BUSY 0					// Is set if a byte is in transmission
#define RX_BUSY 1					// Is set if a byte is being received

// TODO: Framing error handling
//#define RX_FRAME_ERR 2		// Is set if the stop byte isn't a 1

#endif
