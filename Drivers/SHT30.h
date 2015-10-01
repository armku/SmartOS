#ifndef _SHT30_H_
#define _SHT30_H_

#include "I2C.h"

// 光强传感器
class SHT30
{
public:
    I2C* IIC;		// I2C通信口
	byte Address;	// 设备地址

    SHT30();
    virtual ~SHT30();

	void Init();
	ushort ReadSerialNumber();
	ushort ReadStatus();
	ushort ReadTemperature();
	ushort ReadHumidity();

private:
	bool Write(ushort cmd);
	ushort WriteRead(ushort cmd);
	uint ReadAndCrc(ushort cmd);	// 同时读取温湿度并校验Crc
};

#endif
