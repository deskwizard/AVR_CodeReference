/*
 * 		Deskwizard's AVR Code Reference for USART
 *
 *		File: USART.h 		->			Deskwizard's AVR Code Reference for USART (header)
 */

#ifndef _DW_CODE_REFERENCE_USART
#define  _DW_CODE_REFERENCE_USART

void USARTInit(void);
void USARTSendChar(int8_t _char);
uint8_t USARTDataReady(void);
uint8_t USARTReceive(void);

#define USART_UBBR_VALUE ((((F_CPU / 16) + (USART_BAUDRATE / 2)) / (USART_BAUDRATE)) - 1)

#endif //  _DW_CODE_REFERENCE_USART
