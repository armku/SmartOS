﻿#include "Type.h"
#include "Buffer.h"
#include "List.h"

List::List(int size)
{
	_Arr	= Arr;
	_Count	= 0;
	_Capacity	= ArrayLength(Arr) >> 2;
}

//List::List(T* items, uint count) { Set(items, count); }

int List::Count() const { return _Count; }

// 添加单个元素
void List::Add(void* item)
{
	CheckCapacity(_Count + 1);

	_Arr[_Count++]	= item;
}

// 添加多个元素
void List::Add(void** items, uint count)
{
	//assert(items, "items");
	//assert(count, "count");
	if(!items || !count) return;

	CheckCapacity(_Count + count);

	while(count--) _Arr[_Count++]	= items++;
}

// 删除指定位置元素
void List::RemoveAt(uint index)
{
	int len = _Count;
	if(len <= 0 || index >= len) return;

	// 复制元素，最后一个不用复制
	int remain	= len - 1 - index;
	if(remain)
	{
		len	= remain * sizeof(void*);
		Buffer(&_Arr[index], len).Copy(0, &_Arr[index + 1], len);
	}
	_Count--;
}

// 删除指定元素
void List::Remove(const void* item)
{
	int index = FindIndex(item);
	if(index >= 0) RemoveAt(index);
}

int List::FindIndex(const void* item)
{
	for(int i=0; i<_Count; i++)
	{
		if(_Arr[i] == item) return i;
	}

	return -1;
}

// 释放所有指针指向的内存
List& List::DeleteAll()
{
	for(int i=0; i < _Count; i++)
	{
		if(_Arr[i]) delete (int*)_Arr[i];
	}

	return *this;
}

void List::Clear()
{
	_Count = 0;
}

/*// 返回内部指针
const void** List::ToArray() const
{
	return _Arr;
}*/

// 重载索引运算符[]，返回指定元素的第一个
void* List::operator[](int i) const
{
	if(i<0 || i>=_Count) return nullptr;

	return _Arr[i];
}

void*& List::operator[](int i)
{
	if(i<0 || i>=_Count)
	{
		static void* dummy;
		return dummy;
	}

	return _Arr[i];
}
