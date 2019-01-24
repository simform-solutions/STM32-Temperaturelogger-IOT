#include "ds1307rtc.h";

void TM_DS1307_Init(I2C_TypeDef* I2Cx) {


    I2C_InitTypeDef I2C_InitStructure;

    GPIO_InitTypeDef GPIO_InitStructure;



    if(I2Cx == I2C1){


    	I2C_Cmd(I2C1,ENABLE);



    	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);



    	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //PB.6=SCL; PB.7=SDA

    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;

    	GPIO_Init(GPIOB, &GPIO_InitStructure);



    	I2C_InitStructure.I2C_Mode = I2C_Mode_SMBusHost;

    	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

    	I2C_InitStructure.I2C_OwnAddress1 = 0x00; // address for this device

    	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;

    	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    	I2C_InitStructure.I2C_ClockSpeed = 100000 ;

    	I2C_Init(I2C1, &I2C_InitStructure);


}



    else if (I2Cx == I2C2){


    	I2C_Cmd(I2C2,ENABLE);



    	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);



    	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //PB.10=SCL; PB.11=SDA

    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;

    	GPIO_Init(GPIOB, &GPIO_InitStructure);



    	I2C_InitStructure.I2C_Mode = I2C_Mode_SMBusHost;

    	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

    	I2C_InitStructure.I2C_OwnAddress1 = 0x00; // address for this device

    	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;

    	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    	I2C_InitStructure.I2C_ClockSpeed = 100000;

    	I2C_Init(I2C2, &I2C_InitStructure);


}


}



void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){




    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));



    I2C_GenerateSTART(I2Cx, ENABLE);



    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));



    // Send slave Address for write

    I2C_Send7bitAddress(I2Cx, address, direction);



    if(direction == I2C_Direction_Transmitter){


        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


} else if(direction == I2C_Direction_Receiver){


        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));


}


}



void I2C_write(I2C_TypeDef* I2Cx, uint8_t data){


    I2C_SendData(I2Cx, data);

    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


}



uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){


    uint8_t data;



    I2C_AcknowledgeConfig(I2Cx, ENABLE);



    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );



    data = I2C_ReceiveData(I2Cx);

    return data;


}



uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){


    uint8_t data;

    // disabe acknowledge of received data

    // nack also generates stop condition after last byte received

    // see reference manual for more info

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);



    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );



    data = I2C_ReceiveData(I2Cx);

    return data;


}



void I2C_stop(I2C_TypeDef* I2Cx){


    I2C_GenerateSTOP(I2Cx, ENABLE);


}

int bcdTodec(int val){


    return ((val / 16 * 10) + (val % 16));


}

int decToBcd(int val){


    return ((val / 10 * 16) + (val % 10));


}



void TM_DS1307_SetTime(int y, int m, int d, int w, int h, int mi, int s){


    I2C_start(I2C_DS1307, DS1307_I2C_ADDRESS, I2C_Direction_Transmitter);

    I2C_write(I2C_DS1307, 0x00);



    I2C_write(I2C_DS1307, decToBcd(s));

    I2C_write(I2C_DS1307, decToBcd(mi));

    I2C_write(I2C_DS1307, decToBcd(h));

    I2C_write(I2C_DS1307, decToBcd(w));

    I2C_write(I2C_DS1307, decToBcd(d));

    I2C_write(I2C_DS1307, decToBcd(m));

    I2C_write(I2C_DS1307, decToBcd(y));



    I2C_stop(I2C_DS1307);


}



int TM_DS1307_GetTime(time_unit data_type){


	uint8_t y, m, d, w, h, mi, s;

    I2C_start(I2C_DS1307, DS1307_I2C_ADDRESS, I2C_Direction_Transmitter);

    I2C_write(I2C_DS1307, 0x00);

    I2C_stop(I2C_DS1307);



    I2C_start(I2C_DS1307, DS1307_I2C_ADDRESS, I2C_Direction_Receiver);



    s = bcdTodec(I2C_read_ack(I2C_DS1307));

    mi = bcdTodec(I2C_read_ack(I2C_DS1307));

    h = bcdTodec(I2C_read_ack(I2C_DS1307));

    w = bcdTodec(I2C_read_ack(I2C_DS1307));

    d = bcdTodec(I2C_read_ack(I2C_DS1307));

    m = bcdTodec(I2C_read_ack(I2C_DS1307));

    y = bcdTodec(I2C_read_ack(I2C_DS1307)) + 2000;




    I2C_read_nack(I2C_DS1307);



    if(data_type==second) return s;

    else if (data_type==minute) return mi;

    else if (data_type==hour) return h;

    else if (data_type==day) return w;

    else if (data_type==date) return d;

    else if (data_type==month) return m;

    else if (data_type==year) return y;


}
