#include "onewire.h"

/* Init OneWire Struct with GPIO information
 * param:
 *   OneWire: struct to be initialized
 *   GPIOx: Base of the GPIO DQ used, e.g. GPIOA
 *   GPIO_Pin: The pin GPIO DQ used, e.g. 5
 */
void OneWire_Init(OneWire_t* OneWireStruct, GPIO_TypeDef* GPIOx, uint32_t GPIO_Pin) {
	OneWireStruct->GPIO = GPIOx;
	OneWireStruct->GPIO_Pin = GPIO_Pin;
}

/* Send reset through OneWireStruct
 * Please implement the reset protocol
 * param:
 *   OneWireStruct: wire to send
 * retval:
 *    0 -> Reset OK
 *    1 -> Reset Failed
 */
uint8_t OneWire_Reset(OneWire_t* OneWireStruct) {

	OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) );
	OneWireStruct->GPIO->BRR = (1 << OneWireStruct->GPIO_Pin);
	OneWireStruct->GPIO->MODER |= ((0b01) << (OneWireStruct->GPIO_Pin * 2) );
	delay(750);  // 750us
	OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) ); // input mode
	delay(100);   //100us
	int result = (OneWireStruct->GPIO->IDR & (1 << OneWireStruct->GPIO_Pin ) );
	delay(400); //400us
	return result == 0 ? 0 : 1;
}

/* Write 1 bit through OneWireStruct
 * Please implement the send 1-bit protocol
 * param:
 *   OneWireStruct: wire to send
 *   bit: bit to send
 */
void OneWire_WriteBit(OneWire_t* OneWireStruct, uint8_t bit) {
	OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) );

	if (bit)
	{
		OneWireStruct->GPIO->BRR = (1 << OneWireStruct->GPIO_Pin);
		OneWireStruct->GPIO->MODER |= ((0b01) << (OneWireStruct->GPIO_Pin * 2) );
		delay(5); //5us
		OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) );
		delay(70); //70us
	}
	else
	{
		OneWireStruct->GPIO->BRR = (1 << OneWireStruct->GPIO_Pin);
		OneWireStruct->GPIO->MODER |= ((0b01) << (OneWireStruct->GPIO_Pin * 2) );
		delay(75);  // 75us;
		OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) );
	}
}

/* Read 1 bit through OneWireStruct
 * Please implement the read 1-bit protocol
 * param:
 *   OneWireStruct: wire to read from
 */
uint8_t OneWire_ReadBit(OneWire_t* OneWireStruct) {
	OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) );

	OneWireStruct->GPIO->BRR = (1 << OneWireStruct->GPIO_Pin);
	OneWireStruct->GPIO->MODER |= ((0b01) << (OneWireStruct->GPIO_Pin * 2) );

	delay(5); //5us;
	OneWireStruct->GPIO->MODER &= ~((0b11) << (OneWireStruct->GPIO_Pin * 2) );
	delay(5); //2us;
	int result = (OneWireStruct->GPIO->IDR & (1 << OneWireStruct->GPIO_Pin ) );
	delay(60); //50us
	return result == 0 ? 0 : 1;
}

/* A convenient API to write 1 byte through OneWireStruct
 * Please use OneWire_WriteBit to implement
 * param:
 *   OneWireStruct: wire to send
 *   byte: byte to send
 */
void OneWire_WriteByte(OneWire_t* OneWireStruct, uint8_t byte) {
	for (int i=0;i<8;i++)
	{
		OneWire_WriteBit(OneWireStruct, byte%2);
		byte /= 2;
	}
}

/* A convenient API to read 1 byte through OneWireStruct
 * Please use OneWire_ReadBit to implement
 * param:
 *   OneWireStruct: wire to read from
 */
uint8_t OneWire_ReadByte(OneWire_t* OneWireStruct) {
	uint8_t result = 0;
	int p = 1;
	for (int i=0;i<8;i++)
	{
		result += p*OneWire_ReadBit(OneWireStruct);
		p*=2;
	}
	return result;
}

/* Send ROM Command, Skip ROM, through OneWireStruct
 * You can use OneWire_WriteByte to implement
 */
void OneWire_SkipROM(OneWire_t* OneWireStruct) {
	OneWire_WriteByte(OneWireStruct, 0xcc);
}
