/*#include "stm32l476xx.h"
#include "core_cm4.h"
#include "GPIO_init.h"
#include "config.h"
extern void delay();

void init_LCD();
void write_to_LCD(int, int);

int pos=0;

void SysTick_Handler(void)
{
	write_to_LCD(0x01,1);

	int offset = 0x80;
	write_to_LCD(offset+pos,1);
	write_to_LCD(0b00110001, 0);
	if(pos == 0xf)
		pos = 0x40;
	else if (pos == 0x4f)
		pos = 0;
	else
		pos++;
	write_to_LCD(offset+pos,1);
	write_to_LCD(0b00110011, 0);

}

void init_LCD()
{
	write_to_LCD(0x38,1);
	write_to_LCD(0x06,1);
	write_to_LCD(0x0c,1);
	write_to_LCD(0x01,1);
	write_to_LCD(0x80,1);
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
	init_LCD();
	SysTick_Config(1300000);
	while(1){}
}*/
