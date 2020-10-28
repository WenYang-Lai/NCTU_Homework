#ifndef __CONFIG__
#define __CONFIG__

#include "stm32l476xx.h"
#include "core_cm4.h"


int set_reg(int reg, unsigned int mask, int val, unsigned int bit_low)
{
	reg = reg - (reg&mask)+(val<<bit_low);
	return reg;
}

void SystemClock_config()
{
		RCC->CR |= RCC_CR_MSIRGSEL;
		RCC->CR = set_reg(RCC->CR, RCC_CR_MSIRANGE, 6, RCC_CR_MSIRANGE_Pos);
		while(RCC->CR & RCC_CR_MSIRDY == 0); //wait ready
		RCC->CR |= RCC_CR_MSION;// enable MSI clock
		//RCC->CR = set_reg(RCC->CR, RCC_CR_PLLON, 0, RCC_CR_PLLON_Pos);//disable PLL
		//while(RCC->CR & RCC_CR_PLLRDY == 1);//wait PLL clear
		RCC->PLLCFGR = set_reg(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, 1, RCC_PLLCFGR_PLLSRC_Pos);
		RCC->PLLCFGR = set_reg(RCC->PLLCFGR, RCC_PLLCFGR_PLLN, 20, RCC_PLLCFGR_PLLN_Pos);
		RCC->PLLCFGR = set_reg(RCC->PLLCFGR, RCC_PLLCFGR_PLLM, 3, RCC_PLLCFGR_PLLM_Pos);
		RCC->PLLCFGR = set_reg(RCC->PLLCFGR, RCC_PLLCFGR_PLLR, 0, RCC_PLLCFGR_PLLR_Pos);
		RCC->CR = set_reg(RCC->CR, RCC_CR_PLLON, 1, RCC_CR_PLLON_Pos);//enable PLL
		while(RCC->CR & RCC_CR_PLLRDY == 0);
		RCC->PLLCFGR = set_reg(RCC->PLLCFGR, RCC_PLLCFGR_PLLREN, 1, RCC_PLLCFGR_PLLREN_Pos);
		RCC->CFGR = set_reg(RCC->CFGR, RCC_CFGR_SW, 3, RCC_CFGR_SW_Pos);

}
void GPIO_AF()
{
	GPIOB->MODER = set_reg(GPIOB->MODER, GPIO_MODER_MODE0, 10, GPIO_MODER_MODE0_Pos);// AF output
	GPIOB->AFR[0]= set_reg(GPIOB->AFR[0], GPIO_AFRL_AFSEL0, 2, GPIO_AFRL_AFSEL0_Pos);// AF2
}


void InitializeTimer(int _ARR, int _PSC)
{
		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
		set_reg(TIM3->CR1, TIM_CR1_DIR , 1, TIM_CR1_DIR_Pos);//down counter
		TIM3->ARR = _ARR;//Reload value
		TIM3->PSC = _PSC;//Prescalser
		TIM3->EGR = TIM_EGR_UG;//Reinitialize the counter
		TIM3->CR1 = set_reg(TIM3->CR1,TIM_CR1_CEN, 1, TIM_CR1_CEN_Pos );

}

void PWM(int _duty)
{
	TIM3->CCER = set_reg(TIM3->CCER, TIM_CCER_CC3E, 1, TIM_CCER_CC3E_Pos);//enable ch3
	TIM3->CCMR2 = set_reg(TIM3->CCMR2, TIM_CCMR2_OC3M, 0b0110, TIM_CCMR2_OC3M_Pos );
	TIM3->CCR3 = set_reg (TIM3->CCR3, TIM_CCR3_CCR3, _duty, TIM_CCR3_CCR3_Pos);
}

#endif
