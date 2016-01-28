/*
 * 		Deskwizard's AVR Code Reference
 *
 *		File: CodeReference.h 		->			Deskwizard's AVR Code Reference (header)
 */

#ifndef _DW_CODE_REFERENCE_H
#define _DW_CODE_REFERENCE_H

#include <stdint.h>						// Include standardized integer types
#include "project_config.h"			// Include product configuration

extern volatile uint32_t ms_value;

void InitCodeReference(void);

uint32_t currentMs(void);

#define Timer0A 0
#define Timer0B 1
#define Timer1A 2
#define Timer1B 3

#if defined (CodeReferenceCounter_Timer0A)
	#define ISRSelect Timer0A
	#define ISR_CodeReference ISR(TIMER0_COMPA_vect)
	#define CR_Prescaler 64
	#define CR_Prescaler_Mask ((1 << CS01) | (1 << CS00))
#elif defined (CodeReferenceCounter_Timer0B)
	#define ISRSelect Timer0B
	#define ISR_CodeReference ISR(TIMER0_COMPB_vect)

#elif defined (CodeReferenceCounter_Timer1A)
	#define ISRSelect Timer1A
	#define ISR_CodeReference ISR(TIMER1_COMPA_vect)
#elif defined (CodeReferenceCounter_Timer1B)
	#define ISRSelect Timer1B
	#define ISR_CodeReference ISR(TIMER1_COMPB_vect)
#endif

#endif  //  _DW_CODE_REFERENCE_H
