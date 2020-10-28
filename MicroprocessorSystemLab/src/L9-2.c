/*#include "stm32l476xx.h"
#include "core_cm4.h"
#include "GPIO_init.h"
#include "config.h"
extern void delay();
extern int button_input();

void init_LCD();
void write_to_LCD(int, int);

char str[] = "I Love shiro chan.";

int mode = 0;
int pos=0;

void SysTick_Handler(void)
{
	int offset = 0x80;
	if (mode == 0)
	{
		write_to_LCD(0x01,1);

		write_to_LCD(offset+pos,1);
		write_to_LCD(0b00000000, 0);
		if(pos == 0xf)
			pos = 0x40;
		else if (pos == 0x4f)
			pos = 0;
		else
			pos++;
		write_to_LCD(offset+pos,1);
		write_to_LCD(0b00000001, 0);
	}
	else
	{
		if (str[pos] == '\0')
		{
			write_to_LCD(0x01,1);
			pos = 0;
		}
		else
		{
			if (pos < 16)
			{
				write_to_LCD(offset+pos,1);
				write_to_LCD(str[pos], 0);
			}
			else
			{
				write_to_LCD(offset+pos+0x30,1);
				write_to_LCD(str[pos], 0);
			}
			pos++;
		}
	}
}

void init_LCD()
{
	write_to_LCD(0x38,1);
	write_to_LCD(0x06,1);
	write_to_LCD(0x0c,1);
	write_to_LCD(0x01,1);
	// CG RAM set
	write_to_LCD(0x40,1);
	write_to_LCD(0b00100,0);

	write_to_LCD(0x41,1);
	write_to_LCD(0b01110,0);

	write_to_LCD(0x42,1);
	write_to_LCD(0b11001,0);

	write_to_LCD(0x43,1);
	write_to_LCD(0b11000,0);

	write_to_LCD(0x44,1);
	write_to_LCD(0b01100,0);

	write_to_LCD(0x45,1);
	write_to_LCD(0b00110,0);

	write_to_LCD(0x46,1);
	write_to_LCD(0b00110,0);

	write_to_LCD(0x47,1);
	write_to_LCD(0b00011,0);
	// end 1
	write_to_LCD(0x48,1);
	write_to_LCD(0b00100,0);

	write_to_LCD(0x49,1);
	write_to_LCD(0b01110,0);

	write_to_LCD(0x4A,1);
	write_to_LCD(0b10011,0);

	write_to_LCD(0x4B,1);
	write_to_LCD(0b00011,0);

	write_to_LCD(0x4C,1);
	write_to_LCD(0b00110,0);

	write_to_LCD(0x4D,1);
	write_to_LCD(0b00110,0);

	write_to_LCD(0x4E,1);
	write_to_LCD(0b01100,0);

	write_to_LCD(0x4F,1);
	write_to_LCD(0b11000,0);

}

void write_to_LCD(int data, int is_cmd)
{
	//RS: PC0 RW: PC1
	if (is_cmd)
		GPIOC->ODR = 0b000;
	else
		GPIOC->ODR = 0b001;
	GPIOB->ODR = data;

	GPIOC->BSRR = (1<<2);
	if (data == 0x01 || data == 0x02)
		delay(8000);  //2.5ms
	else
		delay(400);
	GPIOC->BRR = (1<<2);
	if (data == 0x01 || data == 0x02)
		delay(8000);  //2.5ms
	else
		delay(400);
}

int main()
{
	GPIO_init_LCD();
	GPIO_init_button();
	init_LCD();
	SysTick_Config(1300000);
	while(1)
	{
		if (button_input() == 0)
		{
			write_to_LCD(0x01,1);
			mode ^= 1;
			pos = 0;
		}
	}
}*/
