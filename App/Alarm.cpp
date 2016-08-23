#include "Alarm.h"

#define Int_Max  2147483647

class AlarmConfig :public ConfigBase
{
public:
	byte Length;
	AlarmDataType Data[20];
	byte TagEnd;

	AlarmConfig();
	virtual void Init();
};

AlarmConfig::AlarmConfig()
{
	_Name = "AlarmCf";
	_Start = &Length;
	_End = &TagEnd;
	Init();
}

void AlarmConfig::Init()
{
	Buffer(Data, sizeof(Data)).Clear();
}

/************************************************/

Alarm::Alarm()
{
	AlarmTaskId = 0;
}

bool Alarm::AlarmSet(const Pair& args, Stream& result)
{
	debug_printf("AlarmSet\r\n");
	AlarmDataType data;
	data.Enable = false;

	Buffer buf = args.Get("alarm");

	Stream ms(buf);
	if (buf.Length() < 7)
	{
		debug_printf("��������\r\n");
		result.Write((byte)0);
		return false;
	}

	byte Id = 0xff;
	Id = ms.ReadByte();
	if (Id > 20)
	{
		debug_printf("Index����\r\n");
		result.Write((byte)0);
		return false;
	}

	data.Enable = ms.ReadByte();

	byte type = ms.ReadByte();
	data.Type.Init(type);

	data.Hour = ms.ReadByte();
	data.Minutes = ms.ReadByte();
	data.Seconds = ms.ReadByte();

	if (data.Hour > 23 || data.Minutes > 59 || data.Seconds > 59)return false;

	// Buffer buf2(data.Data, sizeof(data.Data));
	// auto len = ms.ReadArray(buf2);

	Buffer buf2(data.Data, sizeof(data.Data));
	Buffer buf3(buf.GetBuffer() + ms.Position() , buf.Length() - ms.Position());
	buf2 = buf3;

	debug_printf("%d/%d/%dִ��bs��",data.Hour,data.Minutes,data.Seconds);
	buf2.Show(true);

	if (SetCfg(Id, data))
	{
		result.Write((byte)1);
		return true;
	}
	else
	{
		result.Write((byte)0);
		return false;
	}
}

bool Alarm::AlarmGet(const Pair& args, Stream& result)
{
	debug_printf("AlarmGet\r\n");
	AlarmConfig cfg;
	cfg.Load();

	result.Write((byte)ArrayLength(cfg.Data));		// д�볤��
	for (AlarmDataType &x : cfg.Data)
	{
		Buffer bs(&x.Enable, sizeof(AlarmDataType));
		result.WriteArray(bs);
	}
	Buffer(result.GetBuffer(), result.Position()).Show(true);

	return true;
}

bool Alarm::SetCfg(byte id, AlarmDataType& data)
{
	AlarmConfig cfg;
	cfg.Load();

	Buffer bf(&data, sizeof(AlarmDataType));
	Buffer bf2(&cfg.Data[id].Enable, sizeof(AlarmDataType));
	bf2 = bf;

	cfg.Save();
	// �޸Ĺ���Ҫ���һ��Task��ʱ��	// ȡ���´ζ��������¼���
	NextAlarmIds.Clear();
	Start();
	return true;
}

bool Alarm::GetCfg(byte id, AlarmDataType& data)
{
	AlarmConfig cfg;
	cfg.Load();

	Buffer bf(&data, sizeof(AlarmDataType));
	Buffer bf2(&cfg.Data[id].Enable, sizeof(AlarmDataType));
	bf = bf2;
	return true;
}

int Alarm::CalcNextTime(AlarmDataType& data)
{
	debug_printf("CalcNextTime :");
	auto now = DateTime::Now();
	byte type = data.Type.ToByte();
	byte week = now.DayOfWeek();
	int time;
	if (type & 1 << week)	// ����
	{
		DateTime dt(now.Year, now.Month, now.Day);
		dt.Hour = data.Hour;
		dt.Minute = data.Minutes;
		dt.Second = data.Seconds;
		if (dt > now)
		{
			time = (dt - now).Ms;
			debug_printf("%d\r\n", time);
			return time;		// �������ӻ�û��
		}
	}
	debug_printf("max\r\n");
	return Int_Max;
}

int ToTomorrow()
{
	auto dt = DateTime::Now();
	int time = (24 - dt.Hour - 1) * 3600000;		// ʱ-1  ->  ms
	time += ((60 - dt.Minute - 1) * 60000);			// ��-1  ->  ms
	time += ((60 - dt.Second) * 1000);				// ��	 ->  ms
	// debug_printf("ToTomorrow : %d\r\n", time);
	return time;
}

byte Alarm::FindNext(int& nextTime)
{
	debug_printf("FindNext\r\n");
	AlarmConfig cfg;
	cfg.Load();

	int miniTime = Int_Max;
	int tomorrowTime = ToTomorrow();

	int times[ArrayLength(cfg.Data)];
	for (int i = 0; i < ArrayLength(cfg.Data); i++)
	{
		times[i] = Int_Max;
		if (!cfg.Data[i].Enable)continue;
		int time = CalcNextTime(cfg.Data[i]);	// ������Ч�Ķ��������
		times[i] = time;

		if (time < miniTime)miniTime = time;	// �ҳ���Сʱ��
	}

	if (miniTime != Int_Max)
	{
		for (int i = 0; i < ArrayLength(cfg.Data); i++)
		{
			if (times[i] == miniTime)
			{
				NextAlarmIds.Add(i);
				debug_printf("������һ�����ӵ�id %d\r\n",i);
			}
		}
		nextTime = miniTime;
		debug_printf("��һ������ʱ����%dMs��\r\n",nextTime);
	}
	else
	{
		// �����Сֵ��Ч   ֱ������������һ��
		// nextTime = ToTomorrow();
		debug_printf("����û��������������ʱ��Ϊ����\r\n");
		nextTime = tomorrowTime;
		NextAlarmIds.Clear();
	}

	return NextAlarmIds.Count();
}

void Alarm::AlarmTask()
{
	debug_printf("AlarmTask");
	// ��ȡ��ʱ������
	AlarmDataType data;
	auto now = DateTime::Now();
	now.Ms = 0;
	for (int i = 0; i < NextAlarmIds.Count(); i++)
	{
		byte NextAlarmId = NextAlarmIds[i];
		GetCfg(NextAlarmId, data);

		DateTime dt(now.Year, now.Month, now.Day);
		dt.Hour = data.Hour;
		dt.Minute = data.Minutes;
		dt.Second = dt.Second;
		dt.Ms = 0;
		if (dt == now)
		{
			// ִ�ж���   DoSomething(data);
			debug_printf("  DoSomething:   ");
			// ��һ���ֽ� ��Ч���ݳ��ȣ��ڶ����ֽڶ������ͣ�����������
			byte len = data.Data[0];
			if (len <= 10)
			{
				ByteArray bs((const void*)data.Data[1], len);
				bs.Show(true);

				auto type = (int)data.Data[1];
				AlarmActuator* acttor;
				if (dic.TryGetValue(type, acttor))
				{
					acttor->Actuator(bs);
				}
			}
			else
			{
				debug_printf("��Ч����\r\n");
			}
		}
	}
	NextAlarmIds.Clear();

	// �ҵ���һ����ʱ��������ʱ��
	FindNext(NextAlarmMs);
	if (NextAlarmIds.Count() != 0)
		Sys.SetTask(AlarmTaskId, true, NextAlarmMs);
	else
		Sys.SetTask(AlarmTaskId, false);
}

void Alarm::Start()
{
	debug_printf("Alarm::Start\r\n");
	if (!AlarmTaskId)AlarmTaskId = Sys.AddTask(&Alarm::AlarmTask, this, -1, -1, "AlarmTask");
	Sys.SetTask(AlarmTaskId, true);
}

void Alarm::Register(byte type, AlarmActuator* act)
{
	dic.Add((int)type, act);
}