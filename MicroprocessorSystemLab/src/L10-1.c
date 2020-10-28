#include "config.h"
#include "core_cm4.h"
#include "stm32l476xx.h"
#include "string.h"
#include "GPIO_init.h"
#include "USART.h"

extern int button_input();

char message[128] = "Hello World!";

void ADC1_2IRQn_Handler(void){}
int main()
{
	GPIO_init_button();
	GPIO_init_USART1();
	USART1_init();

	while (1)
	{
		if (button_input() == 0)
		{
			send_string(message);
			send_string("\r\n");
		}
	}
}
