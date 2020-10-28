/*#include "config.h"
#include "core_cm4.h"
#include "stm32l476xx.h"
#include "string.h"
#include "GPIO_init.h"
#include "USART.h"
#include "ADC.h"

extern int button_input();
extern char num[];

int main()
{
	GPIO_init_button();
	GPIO_init_analog();
	ADC_config();
	ADC_enable_interrupt();
	GPIO_init_USART1();
	USART1_init();

	while (1)
	{
		if (button_input() == 0)
		{
			send_string(num);
			send_string("\r\n");
		}
	}
}*/
