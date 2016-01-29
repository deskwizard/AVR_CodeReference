/*
 * 		Deskwizard's AVR Code Reference for USART
 *
 *		File: USART.c 		->			Deskwizard's AVR Code Reference for USART (source)
 *
 *		References used:
 *				->			ATtiny 2313a/4313 datasheet
 *				->			Atmel application note # AVR306 - Using the AVR UART in C
 *				->			Dean Camera's excellent tutorials on USART
 *								->		http://www.fourwalledcubicle.com/AVRArticles.php
 *				->			Doug Brown's blog post on interrupt-safe ring buffers on microcontrollers
 *								->		http://www.downtowndougbrown.com/2013/01/microcontrollers-interrupt-safe-ring-buffers/
 */

#include <avr/io.h>							// Include AVR IO definitions
#include <avr/interrupt.h>				// Include AVR Interrupts definitions
#include <stdlib.h>							// Include standard functions (itoa(), ltoa(), etc...)
#include "USART.h"							// Include USART Reference Code definitions

#define RX_BUFFER_SIZE   32 			// 31 bytes reception buffer
#define TX_BUFFER_SIZE   64 			// 63 bytes transmit buffer

typedef uint8_t ring_pos_t;

volatile ring_pos_t RX_Buffer_head;
volatile ring_pos_t RX_Buffer_tail;
volatile char RX_Buffer[RX_BUFFER_SIZE];

volatile ring_pos_t TX_Buffer_head;
volatile ring_pos_t TX_Buffer_tail;
volatile char TX_Buffer[TX_BUFFER_SIZE];

//*********************************************************************************

// Adds a byte to the transmission buffer
int TX_Buffer_add(char c) {
	ring_pos_t next_head = (TX_Buffer_head + 1) % TX_BUFFER_SIZE;

	if (next_head != TX_Buffer_tail) {
		// TX buffer not full, add to buffer
		TX_Buffer[TX_Buffer_head] = c;
		TX_Buffer_head = next_head;
		return 0;
	} else {		// Buffer is full
		return -1;
	}
}

// Retrieves a byte from the transmission buffer
int TX_Buffer_retrieve(void) {
	if (TX_Buffer_head != TX_Buffer_tail) {
		int c = TX_Buffer[TX_Buffer_tail];
		TX_Buffer_tail = (TX_Buffer_tail + 1) % TX_BUFFER_SIZE;
		return c;
	} else {		// Buffer is empty
		return -1;
	}
}

// Adds a byte to the reception buffer
int RX_Buffer_add(char c) {
	ring_pos_t next_head = (RX_Buffer_head + 1) % RX_BUFFER_SIZE;

	if (next_head != RX_Buffer_tail) {
		// RX buffer not full, add char to buffer
		RX_Buffer[RX_Buffer_head] = c;
		RX_Buffer_head = next_head;
		return 0;
	} else {		// Buffer is full
		return -1;
	}
}

// Retrieves a byte from the reception buffer
int RX_Buffer_retrieve(void) {
	if (RX_Buffer_head != RX_Buffer_tail) {
		int c = RX_Buffer[RX_Buffer_tail];
		RX_Buffer_tail = (RX_Buffer_tail + 1) % RX_BUFFER_SIZE;
		return c;
	} else {		// Buffer is empty
		return -1;
	}
}

// ********************************** Initialization function **********************************/

void USARTInit(uint32_t baudrate) {

	// Disable global interrupts
	cli();

	// Initialize buffers heads and tails
	RX_Buffer_head = 0;
	RX_Buffer_tail = 0;
	TX_Buffer_head = 0;
	TX_Buffer_tail = 0;

	// Set TX pin as output
	DDRD |= (1 << PD1);

	// Enable Receiver and Transmitter (respectively)
	UCSRB = (1 << RXEN) | (1 << TXEN);

	// Configure USART for 8 data bits / 1 stop bit / No parity
	UCSRC = (1 << UCSZ0) | (1 << UCSZ1);

	// Calculate required UBBR value according to requested baudrate
	uint16_t USART_UBBR_VALUE = (uint16_t) ((((F_CPU / 16) + (baudrate / 2))
			/ (baudrate)) - 1);

	// Load calculated UBBR value into UBBRH and URRRL registers
	UBRRH = (USART_UBBR_VALUE >> 8);
	UBRRL = USART_UBBR_VALUE;

	// Enable the USART Receive Complete interrupt ( USART_RXC_vect )
	UCSRB |= (1 << RXCIE);

	// Enable global Interrupts
	sei ();

} // void USARTInit(uint32_t baudrate)

// ********************************** Transmission functions **********************************/

// Adds a single character (int8_t) to the transmission buffer
void USARTSendChar(int8_t byteToSend) {

	TX_Buffer_add(byteToSend);
	UCSRB |= (1 << UDRIE);                    // Enable UDRE interrupt

} // void USARTSendChar(int8_t byteToSend)

// Adds a string to the buffer
void USARTSendStr(char* str) {

	UCSRB |= (1 << UDRIE);                    // Enable UDRE interrupt

	while (*str) // while not \0
	{
		TX_Buffer_add(*str);
		str++;
	}

} // void USARTSendStr(char* str)

// Adds a byte to the transmission buffer
void USARTSend8(uint8_t byteToSend) {
	TX_Buffer_add(byteToSend);				// Add byte to transmission buffer
	UCSRB |= (1 << UDRIE);                    // Enable UDRE interrupt
} // void USARTSendChar(int8_t byteToSend)

// Converts a signed 16 bits integer to a string and adds it to the transmission buffer using USARTSendStr()
void USARTSend16(int16_t data) {
	char str[10];
	itoa(data, str, 10);
	USARTSendStr(str);
} // void USARTSend16(int16_t data)

// Converts a signed 32 bits integer to a string and adds it to the transmission buffer using USARTSendStr()
void USARTSend32(int32_t data) {
	char str[10];
	ltoa(data, str, 10);
	USARTSendStr(str);
} // void UartSend32(int32_t data)


// ********************************** Reception Functions **********************************/

// Returns 1 if there's at least byte waiting to be read in the reception buffer, 0 otherwise
uint8_t USARTDataReady(void) {

	if (RX_Buffer_head != RX_Buffer_tail) {
		return 1;
	} else {
		return 0;
	}
} // uint8_t USARTDataReceived(void)

// Returns the received byte(s) one at a time from the reception buffer
uint8_t USARTReceive(void) {
	return RX_Buffer_retrieve();
} // uint8_t USARTReceive(void)

// ********************************** Interrupt vectors **********************************/

// ***************** USART Data Register Empty Interrupt  (used for transmission) *****************
// Sends a byte if there is a byte left to be sent in the transmit buffer, disables the UDRE interrupt if done transmitting
ISR (USART_UDRE_vect) {

	if (TX_Buffer_head != TX_Buffer_tail) { 	// There's something waiting to be sent, send it.
		UDR = (int8_t) TX_Buffer_retrieve();
	} else {													// The buffer is empty, disable interrupt
		UCSRB &= ~(1 << UDRIE);         		// Disable UDRE interrupt
	}

} // ISR (USART_UDRE_vect)

// ***************** Reception completed Interrupt *****************
// Retrieves a byte from the USART data register and adds it to the reception buffer
ISR (USART_RX_vect) {

	int8_t received_byte = UDR;			// Receive byte
	RX_Buffer_add(received_byte);		// Add byte to reception buffer

} // ISR (USART_RX_vect)
