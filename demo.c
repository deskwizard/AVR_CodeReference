/*
 * 		Deskwizard's AVR Code Reference Demo
 *
 *		File: demo.c 		->			Deskwizard's AVR Code Reference Demo (source)
 *
 *			->		Echoes "Hi" on the hardware USART port and "S" on the software USART port on startup.
 *
 *			-> 	Blinks a LED at "LED_Flash_Interval" millisecond interval.
 *
 *			->		Toggles the red LED on software USART byte reception (first byte on, second byte off, etc...)
 *					and echoes the received byte to the hardware USART.
 *
 *			->		Toggles the green LED on hardware USART byte reception (first byte on, second byte off, etc...)
 *					and echoes the received byte to the software USART
 *
 *		NOTE: 	Software UART Currently hard coded at 19200 bps in SWUSARTInit() function (SWUSART.c).
 *					 *  Sending more than one byte without a delay will hang everything.  *
 */

#include <avr/io.h>							// Include AVR IO definitions
#include "CodeReference.h"				// Include Code Reference general definitions
#include "USART/SWUSART.h"			// Include Software USART Reference Code definitions
#include "USART/USART.h"				// Include USART Reference Code definitions

#define LED_Flash_Interval 50

int main(void) {

	// Initialize Code Reference timer (Millisecond counter)
	InitCodeReference();

	// Initialize hardware USART @ 19200 bps
	USARTInit(19200);

	// Send "Demo started!" followed by "carriage return" (CR) and "line feed" (LF)
	USARTSendStr("Demo started!\r\n");
	USARTSend8('Q');

	// Initialize software UART
	SWUSARTInit();
	SWUSARTSendChar('S');

	// Set LEDs pins as output and set LEDs off
	DDRD |= ((1 << PD3) | (1 << PD4) | (1 << PD5)); 	// Blue | Green | Red
	PORTD |= ((1 << PD3) | (1 << PD4) | (1 << PD5)); 	// Blue | Green | Red

	while (1) { // Loop!
		static uint16_t counter = 0;
		uint8_t receive_byte; // Serial byte received over either software UART or hardware USART

		if (USARTDataReady()) {	// If we received a byte on HW USART, echo it on software UART

			// Toggle green LED
			PORTD ^= (1 << PD4);

			// Receive byte
			receive_byte = USARTReceive();

			// Echo back the received byte back to the computer on software USART
			USARTSendChar(receive_byte);

			// Echo back the received byte back to the computer on software UART
			//SWUSARTSendChar(receive_byte);

		} // if (USARTDataReady())

		if (SWUSARTDataReady()) {// If we received a byte on software UART, echo it on  HW USART

			// Toggle red LED
			PORTD ^= (1 << PD5);

			// Receive byte
			receive_byte = SWUSARTReceive();

			// Echo back the received byte back to the computer on software USART
			USARTSendChar(receive_byte);

		} // if (SWUARTDataReady())

		//  ************************* LED flashing *************************

		uint32_t currentTime = currentMs();  // Take snapshot of current time
		static uint32_t lastFlashMs = 0; // Used to track the time elapse since we last toggle the LED

		if ((uint32_t) (currentTime - lastFlashMs) >= LED_Flash_Interval) { // Has it been "LED_Flash_Interval" milliseconds?
			PORTD ^= (1 << PD3);				// Toggle blue LED
			lastFlashMs = currentTime;		// Save last time LED was toggled

			counter++;

			// DEBUG XXX
			//if (counter <= 30) {
				USARTSendStr("\r\nLED Toggle Count: ");
				USARTSend16(counter);
			//} else {
			//	counter = 55;
			//}

		}  // if = LED_Flash_Interval...

		// *********************** End LED flashing ***********************

	} // while (1)

} // int main(void)
