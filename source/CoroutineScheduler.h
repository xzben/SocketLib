/********************************************************************************
*	文件名称:	CCoroutineScheduler.h											*
*	创建时间：	2015/04/11														*
*	作   者 :	xzben															*
*	文件功能:	c++携程调度器的实现，模拟一个携程									*
*********************************************************************************/
#ifndef __2015_04_11_COROUTINE_SCHEDULER_H__
#define __2015_04_11_COROUTINE_SCHEDULER_H__

#include <cstdint>

class CoSchedulerImplement;
class CCoroutineScheduler;
typedef void(*coroutine_func)(CCoroutineScheduler* pCoSch, void* context);
typedef int32_t CO_ID;

class CCoroutineScheduler
{
public:
	CCoroutineScheduler();
	virtual ~CCoroutineScheduler();

	CO_ID newCoroutine(coroutine_func func, void* context);
	void	resumeCoroutine(CO_ID id);
	void	yieldCoroutine();
private:

	CoSchedulerImplement	*m_pCore;
};
#endif//__2015_04_11_COROUTINE_SCHEDULER_H__