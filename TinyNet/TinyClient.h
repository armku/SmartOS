﻿#ifndef __TinyClient_H__
#define __TinyClient_H__

#include "Sys.h"
#include "TinyMessage.h"

#include "Message\DataStore.h"

// 微网客户端
class TinyClient
{
private:
	TinyController* _control;

public:
	byte	Server;	// 服务端地址
	ushort	Type;	// 设备类型。两个字节可做二级分类
	ByteArray	Password;	// 通讯密码

	ulong	LastActive;	// 最后活跃时间

	TinyClient(TinyController* control);

	void Open();
	void Close();
	
	// 发送消息
	void Send(TinyMessage& msg);
	void Reply(TinyMessage& msg);
	bool OnReceive(TinyMessage& msg);

	// 收到功能消息时触发
	MessageHandler	Received;
	void*			Param;

// 数据区
public:
	DataStore	Store;		// 数据存储区

	void Report(TinyMessage& msg);

private:
	void OnWrite(TinyMessage& msg);
	void OnRead(TinyMessage& msg);

// 常用系统级消息
public:
	// 设置默认系统消息
	void SetDefault();

	// 广播发现系统
	void Discover();
	MessageHandler OnDiscover;
	static bool Discover(Message& msg, void* param);

	// Ping指令用于保持与对方的活动状态
	void Ping();
	MessageHandler OnPing;
	static bool Ping(Message& msg, void* param);

	// 询问及设置系统时间
	static bool SysTime(Message& msg, void* param);

	// 询问系统标识号
	static bool SysID(Message& msg, void* param);

	// 设置系统模式
	static bool SysMode(Message& msg, void* param);

// 通用用户级消息
public:
	byte		Switchs;
	byte		Analogs;
};

#endif
