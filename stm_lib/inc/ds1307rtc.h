#ifndef _TM_STM32F1_DS1307_H_

#define _TM_STM32F1_DS1307_H_

//#include &lt;stdio.h&gt;

//#include &lt;string.h&gt;

#include "stm32f10x_gpio.h";

#include "stm32f10x_i2c.h";

#include "stm32f10x_rcc.h";

#define DS1307_I2C_ADDRESS 0x68 << 1 // DS1307

#define I2C_DS1307 I2C1 //this program using I2C1


typedef enum{


	second=0,

	minute=1,

	hour=2,

	day=3,

	date=4,

	month=5,

	year=5


}time_unit;



void TM_DS1307_Init(I2C_TypeDef* I2Cx);

void TM_DS1307_SetTime(int y, int m, int d, int w, int h, int mi, int s);

int TM_DS1307_GetTime(time_unit data_type);


#endif //_TM_STM32F1_DS1307_H_
