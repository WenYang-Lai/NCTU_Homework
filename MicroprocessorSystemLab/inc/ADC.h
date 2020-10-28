#ifndef __ADC__
#define __ADC__

#include "stm32l476xx.h"
#include "core_cm4.h"
#include "string.h"

#define ADC_CR_DEEPPWD 		(1<<29)
#define ADC_CR_ADVREGEN		(1<<28)
#define	ADC_CR_ADEN			(1)
#define ADC_CR_ADSTART		(1<<2)

#define ADC_ISR_ADRDY 		(1)
#define ADC_ISR_EOC			(1<<2)
#define ADC_ISR_EOS			(1<<3)
#define ADC_ISR_OVR			(1<<4)

#define ADC_CFGR_DMAEN		(1)
#define ADC_CFGR_CONT		(1<<13)
extern void delay();


char num[128]={0};

void ADC_config()
{
	//default is 12bit
	RCC->CCIPR |= (0b11 << 28);  //ADC clock select
	RCC->AHB2ENR |= (1<<13);	// ADC clock enable
	ADC1->CR &= ~ADC_CR_DEEPPWD;	// clear DEEPPWD
	ADC1->CR |= ADC_CR_ADVREGEN;	// turn on voltage
	ADC1->IER |= 1<<2;				//interrupt enable
	delay(200);
	ADC123_COMMON->CCR |= 4<< 8;  	//delay
	ADC123_COMMON->CCR |= 7<< 18; 	//prescale div 16
	ADC1->SQR1 |= 6 << 6;			//channel 6   use PA1
	ADC1->SMPR1 |= (2<<18);				//2 ADC clock cycle

	startADC();
}

void startADC()
{
	while (!(ADC1->ISR & ADC_ISR_ADRDY)) ADC1->CR |= ADC_CR_ADEN;
	ADC1->ISR = (ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR);
	ADC1->CR |= ADC_CR_ADSTART;
}

void ADC_enable_interrupt()
{
	NVIC_EnableIRQ(ADC1_2_IRQn);
}

void ADC1_2IRQn_Handler(void)
{
	while(!(ADC1->ISR & ADC_ISR_EOC));
	itoa(num, ADC1->DR);
	startADC();
	NVIC_ClearPendingIRQ(ADC1_2_IRQn);
}
#endif
