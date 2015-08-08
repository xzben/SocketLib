#include "Thread.h"
#include <thread>
#include "common.h"

void  ThreadFunc(void* param)
{
	CThread* pObj = static_cast<CThread*>(param);
	pObj->m_bIsFinish = false;
	pObj->m_bIsStart = true;
	pObj->run();
	pObj->m_bIsFinish = true;
	pObj->m_bIsStart = false;
}

void CThread::sleep(uint32_t milsecond)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milsecond));
}

CThread::CThread()
: m_bIsFinish(false)
, m_bIsStart(false)
{

}

CThread::~CThread()
{

}

bool CThread::start()
{
	std::thread thr(ThreadFunc, this);
	thr.detach();
	return true;
}
