#ifndef __USART__
#define __USART__

#include "stm32l476xx.h"
#include "string.h"

#define USART_CR1_OVER8 	(1<<15)
#define USART_CR1_PCE  		(1<<10) // priority control enable
#define USART_CR1_PS		(1<<9) 	// odd parity
#define USART_CR1_RE		(1<<2) 	// receive enable
#define USART_CR1_TE		(1<<3)	// transmit enable
#define USART_CR1_UE		(1)		// USART enable

#define USART_CR3_RTSE		(1<<8)
#define USART_CR3_CTSE		(1<<9)
#define USART_CR3_ONEBIT	(1<<11)

#define USART_ISR_TXE		(1<<7)
#define USART_ISR_RXNE		(1<<5)

void USART1_init()
{
	RCC->APB2ENR |= 1 << 14;
	USART1->CR1 = 0;
	USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE);

	USART1->CR2 = 0;  // Set stop bit is 1.

	USART1->CR3 = 0;
	USART1->CR3 |= USART_CR3_ONEBIT;

	USART1->BRR = (4000000/9600);

	USART1->CR1 |= USART_CR1_UE;
}
void send_char(char ch)
{
	while (!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = ch;
}

void send_string(char* str)
{
	int i = 0;
	while (str[i]!='\0')
		send_char(str[i++]);
	//send_char('\r');
	//send_char('\n');
}
#endif
