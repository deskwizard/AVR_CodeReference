/*
 * 		Deskwizard's AVR Code Reference for USART
 *
 *		File: USART.c 		->			Deskwizard's AVR Code Reference for USART (source)
 */

#include <avr/io.h>							// Include AVR IO definitions
#include <avr/interrupt.h>				// Include AVR Interrupts definitions
#include "USART.h"							// Include USART Reference Code definitions

// USART Baudrate configuration
#define USART_BAUDRATE 19200

volatile char ReceivedByte;
volatile uint8_t ReceivedByteFlag = 0;

void USARTInit(void) {
	// Disable global interrupts
	cli();

	// Set TX pin as output
	DDRD |= (1 << PD1);

	UCSRB = (1 << RXEN) | (1 << TXEN); // Turn on the transmission and reception circuitry

	UCSRC = (1 << UCSZ0) | (1 << UCSZ1); // Use 8- bit character sizes

	UBRRH = ( USART_UBBR_VALUE >> 8); // Load upper 8- bits of the baud rate value into the high byte 	of the UBRR register
	UBRRL = USART_UBBR_VALUE; // Load lower 8 - bits of the baud rate value into the low byte of the UBRR register

	UCSRB |= (1 << RXCIE); // Enable the USART Receive Complete interrupt ( USART_RXC )

	sei ();
	// Enable the Global Interrupt Enable flag so that interrupts can be processed
} // void USARTInit(void)

void USARTSendChar(int8_t _char) {
	UDR = _char;
}

uint8_t USARTDataReady(void) {

	return ReceivedByteFlag;

} // uint8_t USARTDataReceived(void)

uint8_t USARTReceive(void) {
	cli();
	uint8_t return_val = ReceivedByte;
	ReceivedByte = 0;
	ReceivedByteFlag = 0;
	sei();
	return return_val;
} // uint8_t USARTReceive(void)

/***********************************************************************************************************************/
// ***************** TX Interrupt  *****************
// ISR (USART_UDRE_vect) {}
// ***************** RX Interrupt *****************
ISR (USART_RX_vect) {
	char _ReceivedByte;
	_ReceivedByte = UDR; // Fetch the received byte value into the variable "ByteReceived"
	ReceivedByte = _ReceivedByte;
	ReceivedByteFlag = 1;
	//PORTD ^= (1 << PD4);				// Toggle green LED
}
