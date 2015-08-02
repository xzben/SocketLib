#include "Thread.h"
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
	m_pCore = nullptr;
}

CThread::~CThread()
{
	SAFE_DELETE(m_pCore);
}

bool CThread::start()
{
	m_pCore = new std::thread(ThreadFunc, this);

	return nullptr != m_pCore;
}
