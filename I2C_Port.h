#ifndef _I2C_PORT_H_
#define _I2C_PORT_H_

#include "Sys.h"
#include "Port.h"


//SCL		开漏复用输出
//SDA		开漏复用输出


class I2C_Port
{
public:

//	I2C_Port(int iic);
//	I2C_Port(I2C_TypeDef* iic);

	// 使用端口和最大速度初始化Spi，因为需要分频，实际速度小于等于该速度
    I2C_Port(int iic = 1, uint speedHz = 9000000);
    virtual ~I2C_Port();

	void SetPin(Pin acl = P0, Pin sda = P0);
	void GetPin(Pin* acl = NULL, Pin* sda = NULL);

	void Open();
	void Close();

//    byte Write(byte data);
//    ushort Write16(ushort data);

private:
	int _iic;
	I2C_TypeDef* _IIC;

    int Speed;  // 速度
    int Retry;  // 等待重试次数，默认200
    int Error;  // 错误次数5
	
	Pin Pins[2];
    char _i2c;
	AlternatePort* SCL;
	AlternatePort* SDA;

};

#endif
