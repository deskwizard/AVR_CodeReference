/*
 * 		Deskwizard's AVR Code Reference for USART
 *
 *		File: USART.h 		->			Deskwizard's AVR Code Reference for USART (header)
 *
 *		References used:
 *				->			ATtiny 2313a/4313 datasheet
 *				->			Atmel application note # AVR306 - Using the AVR UART in C
 *				->			Dean Camera's excellent tutorials on USART
 *								->		http://www.fourwalledcubicle.com/AVRArticles.php
 *				->			Doug Brown's blog post on interrupt-safe ring buffers on microcontrollers
 *								->		http://www.downtowndougbrown.com/2013/01/microcontrollers-interrupt-safe-ring-buffers/
 */

#ifndef _DW_CODE_REFERENCE_USART
#define  _DW_CODE_REFERENCE_USART

// Initialization
void USARTInit(uint32_t baudrate);

// Transmission
void USARTSendChar(int8_t byteToSend);
void USARTSend8(uint8_t byteToSend);

void USARTSendStr(char* str);

// Those use USARTSendStr() as a backend
void USARTSend16(int16_t data);
void USARTSend32(int32_t data);


// Reception
uint8_t USARTDataReady(void);
uint8_t USARTReceive(void);

#endif //  _DW_CODE_REFERENCE_USART
