/*
 * 		Deskwizard's AVR Code Reference
 *		Version 0.1b (27/01/2016-1740)
 *
 *		File: CodeReference_interrupt.c 		->			Deskwizard's AVR Code Reference Interrupt handler(s) (source)
 */

#include <avr/io.h>							// Include AVR IO definitions
#include <avr/interrupt.h>				// Include AVR Interrupts definitions
#include "CodeReference.h"				// Include Code Reference definitions

volatile uint32_t ms_value;				// Current millisecond value

/************************************************************************************************
 * 		Name: 			ISR_CodeReference
 * 		Desc.			Custom Timer Output Compare interrupt vector (Automatically selected in CodeReference.h)
 * 								->		Updates ms_value at each timer compare match register ISR (1 Ms)
 * 		Param.:			None
 * 		Returns:		None
 ************************************************************************************************/
ISR_CodeReference {
	// Copy to local variable so it can be stored in registers since
	// volatile variables must be read from memory on every access
	uint32_t m = ms_value;
	m++;
	ms_value = m;
}
