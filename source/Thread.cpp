#include "Thread.h"
#include <thread>
#include "common.h"

void  ThreadFunc(void* param)
{
	printf("in ThreadFunc\n");
	CThread* pObj = static_cast<CThread*>(param);
	printf("thread is begin running...\n");
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
	//m_pCore = nullptr;
}

CThread::~CThread()
{
	//m_pCore->join();
	//SAFE_DELETE(m_pCore);
}

bool CThread::start()
{
	printf("CThread start....\n");
	//m_pCore = new std::thread(ThreadFunc, this);
	std::thread thr(ThreadFunc, this);
	thr.detach();
	printf("CThread end....\n");
	//return nullptr != m_pCore;
	return true;
}
