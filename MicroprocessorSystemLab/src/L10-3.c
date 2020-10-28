/*#include "config.h"
#include "core_cm4.h"
#include "stm32l476xx.h"
#include "string.h"
#include "GPIO_init.h"
#include "USART.h"
#include "ADC.h"

extern int button_input();
extern char num[];

char cmd[128]={0};
#define prompt 		0
#define waiting		1
#define do_command	2
#define light 		3

void SysTick_Handler(void)
{
	send_string("\r         ");
	send_string("\r");
	send_string(num);
}
int main()
{
	GPIO_init_LED();
	GPIO_init_button();
	GPIO_init_analog();
	ADC_config();
	ADC_enable_interrupt();
	GPIO_init_USART1();
	USART1_init();

	int state = prompt;
	while (1)
	{
		if (USART1->ISR & USART_ISR_RXNE)
		{
			int len = strlen(cmd);
			cmd[len] = USART1->RDR;
			if (cmd[len] == 127 && len >= 1)
			{
				cmd[len-1] = 0;
			}
			if (cmd[len] == '\r' && state == waiting)
			{
				cmd[len]=0;
				send_string("\r\n");
				if (len >= 1)
					state = do_command;
				else
					state = prompt;
			}
			else if(cmd[len] == 'q' && state == light)
			{
				send_string("\r\n");
				SysTick->CTRL = SysTick->CTRL^0b01;
				cmd[0]=0;
				state = prompt;
			}
			else
			{
				cmd[len+1] = 0;
				send_char(cmd[len]);
			}

		}
		if (state == prompt)
		{
			send_string("> ");
			state = waiting;
		}
		else if (state == do_command)
		{
			if (strcmp(cmd ,"showid") == 0)
			{
				send_string("0316025");
				send_string("\r\n");
				state = prompt;
				cmd[0]=0;
			}
			else if (strcmp(cmd, "light") == 0)
			{
				state = light;
				SysTick_Config(2000000);
			}
			else if (strcmp(cmd, "LED on") == 0)
			{
				GPIOB->ODR |= (1<<3);
				send_string("Turn on LED!\r\n");
				state = prompt;
			}
			else if (strcmp(cmd, "LED off") == 0)
			{
				GPIOB->ODR &= ~(1<<3);
				send_string("Turn off LED!\r\n");
				state = prompt;
			}
			else
			{
				send_string("Unknown Command :[");
				send_string(cmd);
				send_string("].\r\n");
				state = prompt;
			}
			cmd[0]=0;
		}

	}
}*/
