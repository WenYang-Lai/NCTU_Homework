	.syntax unified
	.cpu cortex-m4
	.thumb

.data
	direction: 		.word 	0
	button_state:	.word	1<<13
	user_input: 	.word	0
	blink_cycles:	.word	0
	delay_slot:	.word		33333
	debounce_slot: .word	20000
.text
	.global GPIO_init
	.global LED_display
	.global LED_off
	.global button_input
	.global delay
	.global set_debounce_slot
	.global set_delay_slot
	.global	max7219_init
	.global MAX7219Send
	.global change_scan_limit
	.global GPIO_init_MAX7219

	.equ pw,				0b111100000

	.equ left_most,			0b1111100111111
	.equ right_most, 		0b111110011
	.equ RCC_AHB2ENR, 		0x4002104C

	.equ RCC_AHB2ENR, 		0x4002104C
	.equ GPIOA_BASE, 		0x48000000
	.equ GPIOA_MODER, 		0x48000000
	.equ GPIOA_OTYPER, 		0x48000004
	.equ GPIOA_OSPEEDER,	0x48000008
	.equ GPIOA_PUPDR, 		0x4800000C
	.equ GPIOA_BSRR,		0x48000018
	.equ GPIOA_BRR,			0x48000028
	.equ GPIOA_ODR, 		0x48000014

	.equ GPIOB_MODER, 		0x48000400
	.equ GPIOB_OTYPER, 		0x48000404
	.equ GPIOB_OSPEEDER,	0x48000408
	.equ GPIOB_PUPDR, 		0x4800040C
	.equ GPIOB_ODR, 		0x48000414
	.equ GPIOB_IDR, 		0x48000410
	.equ GPIO_BSRR_OFFSET,	0x18
	.equ GPIO_BRR_OFFSET,	0x28

	.equ GPIOC_MODER, 		0x48000800
	.equ GPIOC_OTYPER, 		0x48000804
	.equ GPIOC_OSPEEDER,	0x48000808
	.equ GPIOC_PUPDR, 		0x4800080C
	.equ GPIOC_IDR, 		0x48000810
	.equ GPIOC_BUTTON,		0x1<<13

	.equ DECODE_MODE,		0x09
	.equ DISPLAY_TEST,		0x0f
	.equ INTENSITY,			0x0a
	.equ SCAN_LIMIT,		0x0B
	.equ SHUTDOWN,			0x0c

	.equ DATA,				0x20 //PA5
	.equ LOAD,				0x40 //PA6
	.equ CLOCK,				0x80 //PA7

set_debounce_slot:
	ldr		r1, =debounce_slot
	str		r0, [r1]
	bx		lr

set_delay_slot:
	ldr		r1, =delay_slot
	str		r0, [r1]
	bx		lr
/*	function	*/
button_input:
	push 	{r1, lr}
	ldr		r0, =GPIOC_IDR
	ldr		r1,	[r0]
	ldr		r2,	=GPIOC_BUTTON
	ands	r2, r1
	ldr		r4, =button_state
	ldr		r3,	[r4]
	cmp		r3,	r2
	BEQ		exit_button_input

	/*  debounce	*/
	ldr		r1, =debounce_slot
	ldr		r1,	[r1]
debounce_loop:
	subs 	r1, #1
	bne		debounce_loop
	/*	end	debounce	*/

	ldr		r0, =GPIOC_IDR
	ldr		r1,	[r0]
	ldr		r3,	=GPIOC_BUTTON
	ands 	r3,	r1
	cmp		r2,	r3
	bne		delay
	ldr		r3,	=button_state
	str		r2,	[r3]
	cmp		r2,	#1<<13
	beq		press
	//beq		check_answer

unpress:
	mov		r0, #0
	pop 	{r1, pc}
press:
	mov		r0, #1
	pop 	{r1, pc}

exit_button_input:
	mov		r0, #2
	pop 	{r1, pc}


/*   function    */
GPIO_init_MAX7219:
	/*	clock enable */
	movs 	r0, #0b111
	ldr		r1, =RCC_AHB2ENR
	str		r0, [r1]     //set RCC_AHB2ENR = 1

	movs	r0, #0x5400
	ldr		r1, =GPIOA_MODER
	ldr		r2, [r1]
	and		r2, #0xFFFF03FF
	orrs	r2, r2, r0
	str		r2, [r1]

	movs	r0,	#0x5400
	ldr		r1,	=GPIOA_PUPDR
	ldr		r2, [r1]
	and		r2, #0xFFFF03FF
	orrs	r2, r2, r0
	str		r2,	[r1]

	movs	r0, #0xA800
	ldr		r1, =GPIOA_OSPEEDER
	strh	r0, [r1]

	BX		LR



	/*	GPIOC init */
/*	movs	r0, #0x00
	ldr		r1, =GPIOC_MODER
	ldr		r2, [r1]
	and		r2, #0xF3FFFFFF
	orrs	r2, r2, r0
	str		r2, [r1]

	BX		LR
*/
/*   function    */
LED_display:
	/*		display		*/
	ldr		r0, =GPIOB_ODR
	ldr		r1,	[r0]
	mov		r2, #1
	str		r2,	[r0]
	bx		lr

/*   function    */
LED_off:
	ldr		r0, =GPIOB_ODR
	ldr		r1,	[r0]
	mov		r2, #0
	str		r2,	[r0]
	bx		lr

exit_display:
	bx		lr

/*   function    */
delay:
	PUSH	{r1, LR}
	mov		r1, r0
delay_loop:
	subs 	r1, #1
	bne		delay_loop

	POP		{r1, PC}

max7219_init:

	push {r0, r1, lr}

	ldr r0, =SCAN_LIMIT
	ldr r1, =#0x6
	BL MAX7219Send

	ldr r0, =DECODE_MODE
	mov r1, #0xff
	BL MAX7219Send

	ldr r0, =#DISPLAY_TEST
	ldr r1, =#0x0
	BL MAX7219Send

	ldr r0, =INTENSITY
	ldr r1, =#0x0
	BL MAX7219Send

	ldr r0, =SHUTDOWN
	ldr r1, =#0x1
	BL MAX7219Send
	pop {r0, r1, pc}

MAX7219Send:
	push	{r2,r3,r4,r5,r6,r7,r8}
	lsl r0, r0, #8
	add r0, r0, r1
	ldr r1, =GPIOA_BASE
	ldr r2, =LOAD
	ldr r3, =DATA
	ldr r4, =CLOCK
	ldr r5, =GPIO_BSRR_OFFSET
	ldr r6, =GPIO_BRR_OFFSET
	mov r7, #16//r7 = i

.max7219send_loop:
	mov r8, #1
	sub r9, r7, #1
	lsl r8, r8, r9 // r8 = mask
	str r4, [r1,r6]//HAL_GPIO_WritePin(GPIOA, CLOCK, 0);
	tst r0, r8
	beq .bit_not_set//bit not set
	str r3, [r1,r5]
	b .if_done
.bit_not_set:
	str r3, [r1,r6]
.if_done:
	str r4, [r1,r5]
	subs r7, r7, #1
	bgt .max7219send_loop

	str	r4,	[r1,r6]
	str r2, [r1,r5]
	str	r4,	[r1,r5]
	str r2, [r1,r6]

	/*str r2, [r1,r6]
	str r2, [r1,r5]*/
	pop		{r2,r3,r4,r5,r6,r7,r8}
	BX		LR


change_scan_limit:
	push	{lr}
	mov	r1,	r0
	sub	r1,	#1
	ldr r0, =SCAN_LIMIT
	BL MAX7219Send
	pop		{lr}
