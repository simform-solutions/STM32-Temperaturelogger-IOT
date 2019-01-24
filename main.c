#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include "stm32f10x_usart.h"
#include "stm32f10x.h"
#include "semihosting.h"
#include "misc.h"
#include<stdio.h>
#include<stdlib.h>
#include <math.h>
#include "string.h"
#include "stm32f10x_conf.h"
#include "ff.h"
#include "diskio.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include "ds1307rtc.h"

#define	FF_MAX_SS 512
#define _DS18B20_H
#define IO_DS18B20 GPIO_Pin_0
#define GPIO_DS18B20 GPIOA
#define DS18B20_DQ_High GPIO_SetBits(GPIO_DS18B20,IO_DS18B20)
#define DS18B20_DQ_Low GPIO_ResetBits(GPIO_DS18B20,IO_DS18B20)
FRESULT fr;
FATFS fs;
FIL fil;
char buff[32];
UINT nRead, nWritten;
bool is_valid;
BYTE work[FF_MAX_SS];
char arr[4];
char timestamp[32];
FRESULT open_append(FIL* fp, const char* path);
FRESULT open_append(FIL* fp, /* [OUT] File object to create */
const char* path /* [IN]  File name to be opened */
) {

	/* Opens an existing file. If not exist, creates a new file. */
	fr = f_open(fp, path, FA_WRITE | FA_OPEN_ALWAYS);
	if (fr == 0) {
		/* Seek to end of the file to append data */
		fr = f_lseek(fp, f_size(fp));
		if (fr != 0)
			f_close(fp);
	}
	return fr;
}
typedef struct {
	int8_t integer;
	uint16_t fractional;
	bool is_valid;
} simple_float;

void delay(u32 nCount);
void delay_us(u32 nus);
void delay_ms(u16 nms);

void DS18B20_IO_IN(void);
void DS18B20_IO_OUT(void);
u8 DS18B20_Read_Byte(void);
void DS18B20_Write_Byte(u8 dat);
void DS18B20_Reset(void);
simple_float DS18B20_GetTemp(void);

void usart_init(void)
{
	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	/* USART1 configured as follow:
		- BaudRate = 9600 baud
		- Word Length = 8 Bits
		- One Stop Bit
		- No parity
		- Hardware flow control disabled (RTS and CTS signals)
		- Receive and transmit enabled
		- USART Clock disabled
		- USART CPOL: Clock is active low
		- USART CPHA: Data is captured on the middle
		- USART LastBit: The clock pulse of the last data bit is not output to
			the SCLK pin
	 */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
		USART1 receive data register is not empty */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}
void USARTSend(char *pucBuffer)
{
    while (*pucBuffer)
    {
        USART_SendData(USART1, *pucBuffer++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
        {
        }
    }
}

/*print output on semihosting console*/
void PrintChar(char c)
{
    SH_SendChar(c);
}
/*print float value*/
void printfloat(float f_num, int dplaces)
{
    int p = 1;
    int i;
    for (i = 0; i < dplaces; i++){
        p*=10;
    }
    printf("%d.%d \r\n",(int)floor(f_num),(int)round(p*(f_num-floor(f_num))));

}
/*enable gpio port*/
void RCC_Configuration(void)
{
    SystemInit();//72m
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
}

void delay(u32 nCount)
{
	for(;nCount!=0;nCount--);
}

void delay_us(u32 nus)
{
	 u32 temp;
	 SysTick->LOAD = 9*nus;
	 SysTick->VAL=0X00;
	 SysTick->CTRL=0X01;
	 do
	 {
      temp = SysTick->CTRL;
	 }while((temp&0x01)&&(!(temp&(1<<16))));

	 SysTick->CTRL=0x00;
	 SysTick->VAL =0X00;
}

void delay_ms(u16 nms)
{

	 u32 temp;
	 SysTick->LOAD = 9000*nms;
	 SysTick->VAL=0X00;
	 SysTick->CTRL=0X01;
	 do
	 {
      temp = SysTick->CTRL;
	 }while((temp&0x01)&&(!(temp&(1<<16))));
	 SysTick->CTRL=0x00;
	 SysTick->VAL =0X00;
}

void DS18B20_IO_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin=IO_DS18B20;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIO_DS18B20,&GPIO_InitStructure);
}

void DS18B20_IO_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin=IO_DS18B20;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_DS18B20,&GPIO_InitStructure);
}

u8 DS18B20_Read_Byte(void)
{
	u8 i=0,TempData=0;

	for(i=0;i<8;i++)
	{
		TempData>>=1;

		DS18B20_IO_OUT();
		DS18B20_DQ_Low;
		delay_us(4);
		DS18B20_DQ_High;
		delay_us(10);
		DS18B20_IO_IN();

		if(GPIO_ReadInputDataBit(GPIO_DS18B20,IO_DS18B20)==1)
		{
		   TempData|=0x80;
		}

		delay_us(45);
	}

	return TempData;
}

void DS18B20_Write_Byte(u8 dat)
{
	u8 i=0;
	DS18B20_IO_OUT();

	for(i=0;i<8;i++)
	{
		DS18B20_DQ_Low;
		delay_us(15);

		if((dat&0x01)==1)
		{
			DS18B20_DQ_High;
		}
		else
		{
			DS18B20_DQ_Low;
		}
		delay_us(60);
		DS18B20_DQ_High;

		dat>>=1;
	}
}

void DS18B20_Reset(void)
{
	DS18B20_IO_OUT();
	DS18B20_DQ_Low;
	delay_us(480);
	DS18B20_DQ_High;
	delay_us(480);
}
/*DS18B20 get temprature*/
simple_float DS18B20_GetTemp(void)
{
	simple_float f;
	u8 TL=0,TH=0;
	u16 temp=0;

	DS18B20_Reset();
	DS18B20_Write_Byte(0xCC);
	DS18B20_Write_Byte(0x44);

	delay_ms(800);
	DS18B20_Reset();
	DS18B20_Write_Byte(0xCC);
	DS18B20_Write_Byte(0xBE);

	TL=DS18B20_Read_Byte();//LSB
	TH=DS18B20_Read_Byte();//MSB

	temp=TH;
	float tem =(temp<<8| TL) / powf(2, 4);
    int rest = (tem - (int)tem) * 1000.0;
    f.integer = (int8_t) tem;
    f.fractional = abs(rest);
    f.is_valid = true;

    //int fah = tem * 1.8 + 32;
	 //printf("Fahrenheit: %d F \r\n",fah);
	return f;
}

int main(void)
{
	simple_float temprature;
	float temperature;
	double p,q,r,k;
	float filter;
	RCC_Configuration();
	usart_init(); /*initialize usart */
	/*GSM at commands for configuration gprs service*/
	USARTSend(" AT\r\n");
	delay_ms(2400000);
	USARTSend(" AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");
    delay_ms(2400000);
	USARTSend(" AT+SAPBR=3,1,\"APN\",\"wap.vodafone.co.uk\"\r\n");
	delay_ms(2400000);
	USARTSend(" AT+SAPBR=1,1\r\n");
	delay_ms(2400000);
	USARTSend(" AT+SAPBR=2,1\r\n");
	delay_ms(2400000);
	USARTSend(" AT+HTTPINIT\r\n");
	delay_ms(2400000);
    USARTSend("AT+HTTPPARA=\"CID\",1\r\n");
	delay_ms(2400000);
	USARTSend("AT+HTTPPARA=\"URL\",\"api.thingspeak.com/update\"\r\n");

	TM_DS1307_Init(I2C1); /*initialize DS1307 RTC module*/
    TM_DS1307_SetTime(2018, 3, 13, 4, 2, 39, 0); /*set current date & time for rtc*/
	int count = 0;
	/*initialize sd card*/
	fr = disk_initialize(0);
		char buffer[200];
		fr = f_mount(&fs, (TCHAR const*)arr, 0);
	    if (fr != FR_OK) {
				return fr;
			}
			bool hasFile = true;
		    /*open or create csv file*/
			fr = f_open(&fil,"demo.csv",FA_OPEN_ALWAYS);
			fr = f_mount(&fs, (TCHAR const*)arr, 0);
				/* Open or create a log file and ready to append */
			fr = open_append(&fil, "demo.csv");
			if (fr == FR_OK) {
				f_puts("Date&Timestamp\t Temperature \n", &fil);
				f_close(&fil);
			}

 while(1)
 {
        /*get realtime Date & Time*/
    	int da = TM_DS1307_GetTime(date);
    	int m = TM_DS1307_GetTime(month);
    	int y = TM_DS1307_GetTime(year);
    	int year = 2015+y;
    	int h = TM_DS1307_GetTime(hour);
    	int mi = TM_DS1307_GetTime(minute);
    	int s  = TM_DS1307_GetTime(second);
    	printf("%d/%d/%d, %d:%d:%d \n",da,m,year,h,mi,s);
    	/*Get Temprature*/
    	temprature = DS18B20_GetTemp();
    	/*serial buffer for log date & timestamp and temprature sensor value in sd card*/
    	sprintf(buff, "%d-%d-%d,%d:%d:%d\t %d.%03d\n",da,m,year,h,mi,s, temprature.integer, temprature.fractional);
    	f_mount(&fs, (TCHAR const*)arr, 0); /*mount sd drive*/
    	/* Open or create a log file and ready to append */
    	fr = open_append(&fil, "demo.csv");
    	if (fr == FR_OK) {
    	/* Append a line */
    	f_puts(buff , &fil);
    	/*close file*/
    	f_close(&fil);
        }
    	printf("Temprature:");
    	printf("%d.%03d\n", temprature.integer, temprature.fractional);
    	/*serial buffer for send temprature value to cloud*/
    	sprintf(buff, "%d.%03d",temprature.integer, temprature.fractional);
    	/*gsm at commands for sending temprature sensor data to cloud*/
        USARTSend(" AT+HTTPDATA=62,10000\r\n");
        delay_ms(2400000);
  	    USARTSend("api_key=TXQVQPD307GC5KYO&field1=");
  	    USARTSend(buff);
  	  	USARTSend("\r\n");
  	    USARTSend("api_key=TXQVQPD307GC5KYO&field1=");
  	    USARTSend(buff);
  	    USARTSend("\r\n");
  	    delay_ms(2400000);
  	    USARTSend(" AT+HTTPACTION=1\r\n");
  	    delay_ms(2400000);
    	//Kalman filter
    	p = 125 + 0.125;
    	k = p / (p + 3);
    	filter = 0+k*(temprature.integer-0);
    	p= 1-k*p;
    	char *temp = ( char* ) &filter;
        printf("Filter:");
    	printfloat(filter,2);
    	delay_ms(3800000);
}
}

