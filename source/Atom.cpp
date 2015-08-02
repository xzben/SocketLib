#include "Atom.h"
#include "common.h"
#if (CUR_PLATFROM == PLATFORM_WIN32) 
#include <windows.h>
#endif
/*
*	function : 原子量自加 1
*	parameter: var 要被增加的原子量地址
*	return:
*		WIN32: 返回自增后的值
*/
uint32_t AtomSelfAdd(void * var)
{
#if (CUR_PLATFROM == PLATFORM_WIN32) 
	return InterlockedIncrement((long *)(var)); // NOLINT
#else
	return __sync_add_and_fetch((uint32_t *)(var), 1); // NOLINT
#endif
}
/*
*	function : 原子量自减 1
*	parameter: var 要被操作的原子量地址
*	return:
*		WIN32: 返回自减后的值
*/
uint32_t AtomSelfDec(void * var)
{
#if (CUR_PLATFROM == PLATFORM_WIN32) 
	return InterlockedDecrement((long *)(var)); // NOLINT
#else
	return __sync_add_and_fetch((uint32_t *)(var), -1); // NOLINT
#endif
}
/*
*	function : 原子量增加指定值
*	parameter: var 要被操作的原子量地址， value 要增加的值
*	return:
*		WIN32: 返回增加前的值
*/
uint32_t AtomAdd(void * var, const uint32_t value)
{
#if (CUR_PLATFROM == PLATFORM_WIN32) 
	return InterlockedExchangeAdd((long *)(var), value); // NOLINT
#else
	return __sync_fetch_and_add((uint32_t *)(var), value);  // NOLINT
#endif
}
/*
*	function : 原子量减小指定值
*	parameter: var 要被操作的原子量地址， value 要减小的值
*	return:
*		WIN32: 返回减小前的值
*/
uint32_t AtomDec(void * var, int32_t value)
{
	value = value * -1;
#if (CUR_PLATFROM == PLATFORM_WIN32) 
	return InterlockedExchangeAdd((long *)(var), value); // NOLINT
#else
	return __sync_fetch_and_add((uint32_t *)(var), value);  // NOLINT
#endif
}

/*
*	function : 原子量设置成指定的值
*	parameter: var 要被操作的原子量地址， value 要被设置的值
*	return:
*		WIN32: 返回设置前的值
*/
uint32_t AtomSet(void * var, const uint32_t value)
{
#if (CUR_PLATFROM == PLATFORM_WIN32) 
	::InterlockedExchange((long *)(var), (long)(value)); // NOLINT
#else
	__sync_lock_test_and_set((uint32_t *)(var), value);  // NOLINT
#endif
	return value;
}

/*
*	function : 获取原子量的值
*	parameter: var 要被操作的原子量地址
*	return:
*		WIN32: 原子量的当前值
*/
uint32_t AtomGet(void * var)
{
#if (CUR_PLATFROM == PLATFORM_WIN32) 
	return InterlockedExchangeAdd((long *)(var), 0); // NOLINT
#else
	return __sync_fetch_and_add((uint32_t *)(var), 0);  // NOLINT
#endif
}
