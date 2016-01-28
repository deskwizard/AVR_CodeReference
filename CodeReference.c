/*
 * 		Deskwizard's AVR Code Reference
 *
 *		File: CodeReference.c 		->			Deskwizard's AVR Code Reference  (source)
 */

#include <avr/io.h>							// Include AVR IO definitions
#include <avr/interrupt.h>				// Include AVR Interrupts definitions
#include "CodeReference.h"				// Include Code Reference general definitions

#define MS_OCRxA (uint16_t)((F_CPU/CR_Prescaler)/(1000000/(1000)))			// Calculate required OCR1A value according to F_CPU
extern volatile uint32_t ms_value;																// Current millisecond value

/************************************************************************************************
 * 		Name: 			void InitCodeReference(void)
 * 		Desc.			Initialize Timer millisecond counter on selected timer
 * 								-> See comments inside function
 * 		Param.:			None
 * 		Returns:		None
 ************************************************************************************************/
void InitCodeReference(void) {

	if ((ISRSelect == Timer0A) | (ISRSelect == Timer0B)) {

		TCCR0B = CR_Prescaler_Mask;				// Timer0 Prescaler
		TCCR0B |= (1 << WGM02); 					// Timer0 Mode = CTC w/ OCR1A top
		OCR0A = MS_OCRxA;							// Set OCR1A to required value

		if (ISRSelect == Timer0A) {
			TIMSK = (1 << OCIE0A);					// Enable Interrupt Counter 0 Output Compare A (TIMER0_CMPA_vect)
		} else {
			OCR0B = MS_OCRxA;						// Set OCR0B to required value
			TIMSK = (1 << OCIE0B); 					// Enable Interrupt Counter 0 Output Compare B (TIMER0_CMPB_vect)
		}

	} else if ((ISRSelect == Timer1A) | (ISRSelect == Timer1B)) {

		TCCR1B = CR_Prescaler_Mask; 				// Timer1 Prescaler
		TCCR1B |= (1 << WGM12); 					// Timer1 Mode = CTC w/ OCR1A top
		OCR1A = MS_OCRxA;							// Set OCR1A to required value

		if (ISRSelect == Timer1A) {
			TIMSK = (1 << OCIE1A); 					// Enable Interrupt Counter 1 Output Compare A (TIMER1_CMPA_vect)
		} else {
			OCR1B = MS_OCRxA;						// Set OCR1B to required value
			TIMSK = (1 << OCIE1B); 					// Enable Interrupt Counter 1 Output Compare B (TIMER1_CMPB_vect)
		}
	}

	sei();
	// Enable global interrupts
} // void InitCodeReference(void)

/************************************************************************************************
 * 		Name: 			currentMs(void)
 * 		Desc.			Millisecond return routine
 * 								-> returns current the millisecond timer counter value safely.
 * 		Param.:			None
 * 		Returns:		current_ms (uint32_t)
 ************************************************************************************************/
uint32_t currentMs(void) {
	uint32_t current_ms;
	uint8_t oldSREG = SREG;

	// Disable interrupts while we read ms_value or we might get an
	// inconsistent value (e.g. in the middle of a write to ms_value)
	cli();
	current_ms = ms_value;
	SREG = oldSREG;

	return current_ms;
} // uint32_t currentMs(void)
