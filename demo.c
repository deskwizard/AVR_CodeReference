/*
 * 		Deskwizard's AVR Code Reference Demo
 *		Version 0.1b (27/01/2016-1740)
 *
 *		File: demo.c 		->			Deskwizard's AVR Code Reference Demo (source)
 *
 *			-> Blinks 2 LEDs alternatively at "LED_Flash_Interval" millisecond interval
 */


#include <avr/io.h>							// Include AVR IO definitions
#include "CodeReference.h"				// Include Code Reference definitions

#define LED_Flash_Interval 250

int main(void) {

	// Initialize CodeReference timer (Millisecond counter)
	InitCodeReference();

	// Set LED pin as output
	DDRD |= ((1 << PD4) | (1 << PD5) | (1 << PD6)); 		// Green | Red | Blue
	PORTD |= ((1 << PD4) | (1 << PD5) | (0 << PD6)); 	// Green and Red off (active low), Blue on

	while (1) { // Loop!

		//  **************** LED flashing ****************
		uint32_t currentTime = currentMs();  // Update snapshot of current time
		static uint32_t previousMillis = 0; 	 // Used to track the time elapse since we toggle the LED

		if ((uint32_t) (currentTime - previousMillis) >= LED_Flash_Interval) { 		// Has it been "LED_Flash_Interval" milliseconds?
			PORTD ^= (1 << PD4);				// Toggle green LED
			PORTD ^= (1 << PD6);				// Toggle green LED
			previousMillis = currentTime;	// Save last time LED was toggled
		}
		// ************** end LED flashing **************

	} // end loop

} // int main(void)
