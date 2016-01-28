/*
 * 		Deskwizard's AVR Code Reference Demo
 *
 *		File: demo.c 		->			Deskwizard's AVR Code Reference Demo (source)
 *
 *			-> 	Blinks a LED at "LED_Flash_Interval" millisecond interval.
 *
 *			->		Toggles the red LED on software UART byte reception (first byte on, second byte off, etc...)
 *					and echoes the received byte to the hardware USART.
 *
 *			->		Toggles the green LED on hardware USART byte reception (first byte on, second byte off, etc...)
 *					and echoes the received byte to the software UART
 *
 *		NOTE: 	Software UART Currently fixed at 19200 bps in SWUARTInit().
 *		NOTE:  Hardware USART Currently fixed at 19200 bps in USART.c
 */

#include <avr/io.h>							// Include AVR IO definitions
#include "CodeReference.h"				// Include Code Reference general definitions
#include "USART/USART.h"				// Include USART Reference Code definitions
#include "USART/SWUART.h"			// Include Software UART Reference Code definitions

#define LED_Flash_Interval 250

int main(void) {

	// Initialize Code Reference timer (Millisecond counter)
	InitCodeReference();

	// Initialize hardware USART
	USARTInit();
	USARTSendChar('H');
	USARTSendChar('i');
	USARTSendChar('\r');
	USARTSendChar('\n');

	// Initialize software UART
	SWUARTInit();
	SWUARTSendChar('S');

	// Set LEDs pins as output and set LEDs off
	DDRD |= ((1 << PD3) | (1 << PD4) | (1 << PD5)); 		// Blue | Green | Red
	PORTD |= ((1 << PD3) | (1 << PD4) | (1 << PD5)); 	// Blue | Green | Red

	while (1) { // Loop!
		uint8_t receive_byte; // Serial byte received over either software UART or hardware USART

		if (USARTDataReady()) {		// If we received a byte on HW USART, echo it on software UART

			// Toggle green LED
			PORTD ^= (1 << PD4);

			// Receive byte
			receive_byte = USARTReceive();

			// Echo back the received byte back to the computer on software UART
			SWUARTSendChar(receive_byte);

		} // if (USARTDataReady())


		if (SWUARTDataReady()) {		// If we received a byte on software UART, echo it on  HW USART

			// Toggle red LED
			PORTD ^= (1 << PD5);

			// Receive byte
			receive_byte = SWUARTReceive();

			// Echo back the received byte back to the computer on software USART
			USARTSendChar(receive_byte);

		} // if (SWUARTDataReady())


		//  ************************* LED flashing *************************

		uint32_t currentTime = currentMs();  // Take snapshot of current time
		static uint32_t lastFlashMs = 0; // Used to track the time elapse since we toggle the LED

		if ((uint32_t) (currentTime - lastFlashMs) >= LED_Flash_Interval) { // Has it been "LED_Flash_Interval" milliseconds?
			PORTD ^= (1 << PD3);				// Toggle blue LED
			lastFlashMs = currentTime;	// Save last time LED was toggled
		}  // if = LED_Flash_Interval...

		// *********************** End LED flashing ***********************

	} // while (1)

} // int main(void)
