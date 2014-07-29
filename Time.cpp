#include "Time.h"

#define TIME_Completion_IdleValue 0xFFFFFFFFFFFFFFFFull

#define SYSTICK_MAXCOUNT       SysTick_LOAD_RELOAD_Msk	//((1<<24) - 1)	/* SysTick MaxCount */
#define SYSTICK_ENABLE         SysTick_CTRL_ENABLE_Msk	//     0		/* Config-Bit to start or stop the SysTick Timer */

Func Time_Handler;

Time::Time()
{
	Time_Handler = NULL;
	Ticks = 0;
	NextEvent = TIME_Completion_IdleValue;

	// 准备使用外部时钟，Systick时钟=HCLK/8
	// 48M时，每秒48M/8=6M个滴答，1us=6滴答
	// 72M时，每秒72M/8=9M个滴答，1us=9滴答
	// 96M时，每秒96M/8=12M个滴答，1us=12滴答
	// 120M时，每秒120M/8=15M个滴答，1us=15滴答
	TicksPerSecond = Sys.Clock / 8;		// 每秒的嘀嗒数
	TicksPerMillisecond = TicksPerSecond / 1000;	// 每毫秒的嘀嗒数
	TicksPerMicrosecond = TicksPerSecond / 1000000;	// 每微秒的时钟滴答数

	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;	// 选择外部时钟，每秒有个HCLK/8滴答
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;		// 开启定时器减到0后的中断请求

	// 加载嘀嗒数，72M时~=0x00FFFFFF/9M=1864135us，96M时~=0x00FFFFFF/12M=1398101us
	SysTick->LOAD = SYSTICK_MAXCOUNT - 1;
	SysTick->VAL = 0;
	SysTick->CTRL |= SYSTICK_ENABLE;	// SysTick使能

	NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = (byte)SysTick_IRQn;
    //nvic.NVIC_IRQChannelSubPriority = 0;	// 需要最高级别的中断优先级
#ifdef STM32F10X
    nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic.NVIC_IRQChannelSubPriority = 0x00; // 需要最高级别的中断优先级
#else
    nvic.NVIC_IRQChannelPriority = 0x00;    // 想在中断里使用延时函数就必须让此中断优先级最高
#endif
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    
    //(*(TInterrupt*)TInterrupt::GetInstance()).Activate(SysTick_IRQn, OnHandler, 0);
    //g_Interrupt->Activate
    //(*g_Interrupt).Activate
	//Time_Handler = OnHandler;
	// 再次打开中断，为了设定中断函数
	Interrupt.Activate(SysTick_IRQn, OnHandler, this);
}

Time::~Time()
{
	//Time_Handler = NULL;
    Interrupt.Deactivate(SysTick_IRQn);
    // 关闭定时器
	//SysTick->CTRL &= ~SYSTICK_ENABLE;
}

void Time::OnHandler(ushort num, void* param)
{
	// 累加计数
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG)
	{
		g_Time->Ticks += SysTick->LOAD;
	}
}

void Time::SetCompare(ulong compareValue)
{
    Sys.DisableInterrupts();

    NextEvent = compareValue;

	ulong curTicks = CurrentTicks();

	uint diff;

	// 如果已经超过计划比较值，那么安排最小滴答，准备马上中断
	if(curTicks >= NextEvent)
		diff = 1;
	// 计算下一次中断的间隔，最大为SYSTICK_MAXCOUNT
	else if((compareValue - curTicks) > SYSTICK_MAXCOUNT)
		diff = SYSTICK_MAXCOUNT;
	else
		diff = (uint)(compareValue - curTicks);

	// 把时钟里面的剩余量累加到g_Ticks
	Ticks = CurrentTicks();

	// 重新设定重载值，下一次将在该值处中断
	SysTick->LOAD = diff;
	SysTick->VAL = 0x00;

    Sys.EnableInterrupts();
}

ulong Time::CurrentTicks()
{
	//Sys.DisableInterrupts();

	uint value = (SysTick->LOAD - SysTick->VAL);
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG)
	{
		Ticks += SysTick->LOAD;
	}

    //Sys.EnableInterrupts();

	return Ticks + value;
}

#define STM32_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS 3

void Time::Sleep(uint us)
{
    // 睡眠时间太短
    if(us <= STM32_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS) return ;

	// 自己关闭中断，简直实在找死！
	// Sleep的时候，尽量保持中断打开，否则g_Ticks无法累加，从而造成死循环
	// 记录现在的中断状态
	bool state = Sys.EnableInterrupts();

	// 时钟滴答需要采用UINT64
    ulong maxDiff = us * TicksPerMicrosecond;
    ulong current = CurrentTicks();

	// 减去误差指令周期，在获取当前时间以后多了几个指令
    if(maxDiff <= STM32_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS)
		maxDiff  = 0; 
    else
		maxDiff -= STM32_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS;

	maxDiff += current;

    while(CurrentTicks() <= maxDiff);
	
	// 如果之前是打开中断的，那么这里也要重新打开
	if (!state) Sys.DisableInterrupts();
}
