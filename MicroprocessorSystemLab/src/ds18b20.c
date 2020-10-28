#include "ds18b20.h"

/* Send ConvT through OneWire with resolution
 * param:
 *   OneWire: send through this
 *   resolution: temperature resolution
 * retval:
 *    0 -> OK
 *    1 -> Error
 */
int DS18B20_ConvT(OneWire_t* OneWire, DS18B20_Resolution_t resolution) {
	if (OneWire_Reset(OneWire) == 0)
	{
		int a;
		a =1;
	}
	OneWire_SkipROM(OneWire);
	OneWire_WriteByte(OneWire, 0x44); // convert T;

	int slot = 110000;
	for (int i=0;i<resolution-9; i++)
		slot *= 2;
	delay(slot);

	return 0;
}

/* Read temperature from OneWire
 * param:
 *   OneWire: send through this
 *   destination: output temperature
 * retval:
 *    0 -> OK
 *    1 -> Error
 */
uint8_t DS18B20_Read(OneWire_t* OneWire, char *destination) {

	if (OneWire_Reset(OneWire) == 0)
	{
		int a;
		a =1;
	}
	OneWire_SkipROM(OneWire);
	OneWire_WriteByte(OneWire, 0xbe); // read scratchpad;

	int lsb = OneWire_ReadByte(OneWire);
	int msb = OneWire_ReadByte(OneWire);

	destination[0] = '0';
	destination[1] = '.';
	destination[2] = '0';
	destination[3] = '0';
	destination[4] = '0';
	destination[5] = '\0';

	lsb = lsb >> 1;
	float f = 0.125;
	for (int i=0;i<3;i++)
	{
		if ((lsb & 1))
		{
			if (f == 0.125)
			{
				destination[2] += 1;
				destination[3] += 2;
				destination[4] += 5;
			}
			else if (f == 0.25)
			{
				destination[2] += 2;
				destination[3] += 5;
			}
			else
				destination[2] += 5;
		}
		f *= 2;
		lsb = lsb >> 1;
	}

	int tmp = 1;
	int num = 0;
	for (int i=0;i<4;i++)
	{
		if ((lsb & 1))
			num += tmp;
		tmp *= 2;
		lsb = lsb >> 1;
	}

	for (int i=0; i<3; i++)
	{
		if ((msb & 1))
			num += tmp;
		tmp *= 2;
		msb = msb >> 1;
	}
	char str_num[128];
	itoa(str_num, num);

	int l = strlen(str_num)-1;
	for (int i=5;i>=0;i--)
		destination[i+l] = destination[i];
	l++;
	for (int i=0;i<l;i++)
	{
		destination[i] = str_num[i];
	}

	if ((msb & 1))
	{
		int len = strlen(destination);
		for (int i=len+1; i>0; i--)
			destination[i] = destination[i-1];
		destination[0] = '-';
	}
	return 0;
}

/* Set resolution of the DS18B20
 * param:
 *   OneWire: send through this
 *   resolution: set to this resolution
 * retval:
 *    0 -> OK
 *    1 -> Error
 */
uint8_t DS18B20_SetResolution(OneWire_t* OneWire, DS18B20_Resolution_t resolution) {
	OneWire_Reset(OneWire);
	OneWire_SkipROM(OneWire);
	OneWire_WriteByte(OneWire, 0x4e); // write scratchpad

	OneWire_WriteByte(OneWire, 0x7f);
	OneWire_WriteByte(OneWire, 0x80);
	OneWire_WriteByte(OneWire, ( (0x1f) | ((resolution-9)<<5) ) );

	OneWire_Reset(OneWire);
	OneWire_SkipROM(OneWire);
	OneWire_WriteByte(OneWire, 0x48); // copy scratchpad


	return 0;
}

/* Check if the temperature conversion is done or not
 * param:
 *   OneWire: send through this
 * retval:
 *    1 -> OK
 *    0 -> Not yet
 */
uint8_t DS18B20_Done(OneWire_t* OneWire) {
	return OneWire_ReadBit(OneWire);
}
