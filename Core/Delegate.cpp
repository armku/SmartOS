﻿#include "Type.h"
#include "DateTime.h"
#include "SString.h"

#include "Delegate.h"

/************************************************ Delegate ************************************************/
Delegate::Delegate()
{
	Method	= nullptr;
	Target	= nullptr;
}

Delegate::Delegate(void* func)
{
	Method	= func;
	Target	= nullptr;
}

Delegate::Delegate(void* func, void* target)
{
	Method	= func;
	Target	= target;
}

Delegate::Delegate(Func func)	{ Method	= (void*)func; }
Delegate::Delegate(Action func)	{ Method	= (void*)func; }
Delegate::Delegate(Action2 func){ Method	= (void*)func; }
Delegate::Delegate(Action3 func){ Method	= (void*)func; }

/*void Delegate::Add(Func func)
{
	Method	= (void*)func;
}*/

void Delegate::operator()()
{
	if(!Method) return;

	if(Target)
		((Action)Method)(Target);
	else
		((Func)Method)();
}

void Delegate::operator()(void* arg)
{
	if(!Method) return;

	if(Target)
		((Action2)Method)(Target, arg);
	else
		((Action)Method)(arg);
}

void Delegate::operator()(void* arg, void* arg2)
{
	if(!Method) return;

	if(Target)
		((Action3)Method)(Target, arg, arg2);
	else
		((Action2)Method)(arg, arg2);
}