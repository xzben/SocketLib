/********************************************************************************
*	文件名称:	Thread.h														*
*	创建时间：	2015/03/29														*
*	作   者 :	xzben															*
*	文件功能:	线程功能实现的基类，继承CThread的类都将拥有一个线程					*
*********************************************************************************/
#ifndef __2015_03_29_THREAD_H__
#define __2015_03_29_THREAD_H__

#include "config.h"
#include <thread>

class CThread
{
public:
	CThread();
	~CThread();
	static void sleep(uint32_t milsecond);

	virtual bool start();
	virtual void run() = 0;
	bool isFinish(){ return m_bIsFinish; }
	bool isStart(){ return m_bIsStart; }
private:
	friend void		ThreadFunc(void* param);
	//std::thread		*m_pCore;
	bool			m_bIsFinish;
	bool			m_bIsStart;

};
#endif//__2015_03_29_THREAD_H__
