#include "Thread.h"
#include "Logger.h"


//////////////////////////////////////////////////////////////////////////
//Thread
DWORD __stdcall Thread::ThreadFunc(void *arg)
{
	static volatile int s_RunningThreadNumberCounter = 0;
	s_RunningThreadNumberCounter ++;

	Thread *pThead = (Thread*)arg;
	//调试信息
	Logger::getInstace()->debug("Thread %s is start.\r\nCurrent Thread number[%d]", pThead->m_strThreadName.c_str(), s_RunningThreadNumberCounter);
	//线程开始运行设置运行标志
	pThead->m_ThreadMutex.lock();
	pThead->m_bActive = true;
	pThead->m_bComplete = false;
	pThead->m_ThreadMutex.unlock(); 

	//线程主回调
	pThead->run();

	//线程结束设置运行标志
	pThead->m_ThreadMutex.lock();
	pThead->m_bActive = false;
	pThead->m_bComplete = true;
	pThead->m_ThreadMutex.unlock();

	s_RunningThreadNumberCounter --;
	Logger::getInstace()->debug("Thread[%s] is End.\r\nCurrent Thread Number[%d]", pThead->m_strThreadName.c_str(), s_RunningThreadNumberCounter);

	//如果 对象时不可 Join 的则在线程结束时自动将对象删除
	if( !pThead->isJoinable() )
	{
		SAFE_DELETE(pThead);
	}
	else //如果对象为可 Join 的则 线程结束时只是关闭线程句柄，线程对象由 join 的线程去删除
	{
		::CloseHandle(pThead->m_hThreadHandle);
		pThead->m_hThreadHandle = NULL;
	}

	return 0;
}

void Thread::join()
{
	WaitForSingleObject(m_hThreadHandle, INFINITE);
}

bool Thread::start()
{
	if(m_bActive) //如果已经运行了，直接返回
		return false;

	DWORD dwThreadID;
	m_hThreadHandle = CreateThread(NULL,
		0, 
		(LPTHREAD_START_ROUTINE)ThreadFunc,
		(void*) this, 
		0, 
		&dwThreadID);

	if(NULL == m_hThreadHandle)
	{
		Logger::getInstace()->error("[%s] is Start failded", m_strThreadName.c_str());
		return false;
	}

	return true;
}