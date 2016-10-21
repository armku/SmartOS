﻿#ifndef __RASTER__H__
#define __RASTER__H__

#include "Sys.h"

#pragma pack(push)
#pragma pack(1)

// 光栅组数据部分
struct  RasTriData
{
	byte	Index;		// 光栅组
	byte 	Direction;	// 方向
	ushort	Time;		// AB响应间隔 单位MS
	UInt64  Start;		// 触发时间

	ushort	TimeA;		// 光栅A遮挡时间
	ushort	TimeB;		// 光栅B遮挡时间
	ushort	Count;		// 本地计数

	byte Size() { return sizeof(this[0]); }
};
// 单路光栅部分
struct  FlagData
{
	byte	Count;			//计数
	UInt64  Start;		//触发时间
	ushort  Time;		//光栅遮挡时间

	byte Size() { return sizeof(this[0]); }
};
#pragma pack(pop)

class PulsePort;

// 光栅类  直接对接 TokenClient
class Raster
{
public:
	byte	Index;			// 光栅组索引
	bool	Opened;

	Raster(Pin pinA, Pin pinB, Pin bz = P0);
	~Raster();

	bool Open();

	Delegate<Stream&> OnReport;

private:
	PulsePort*  RasterA;		//每一组光栅分两路
	PulsePort*  RasterB;

	MemoryStream	Cache;		//实际送的数据
	uint    _task;

	FlagData FlagA;				//A标志
	FlagData FlagB;				//B标志
	byte Count;
	//光栅计数
	OutputPort	Buzzer;

	void Init(Pin bz);
	void OnHandlerA(PulsePort& raster);
	void OnHandlerB(PulsePort& raster);
	void LineReport();

	// 上报数据
	void Report();
};

#endif
