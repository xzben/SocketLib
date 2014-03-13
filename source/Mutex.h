#ifndef __2014_03_02_CMUTEX_H__
#define __2014_03_02_CMUTEX_H__

#include "CObject.h"

/*
*	线程资源互斥锁
*/
class Mutex : public NoCopyable
{
public:
	Mutex()
	{
		InitializeCriticalSection( &m_csLock );
	}
	~Mutex()
	{
		DeleteCriticalSection( &m_csLock );
	}
	inline void	lock()
	{
		EnterCriticalSection( &m_csLock );
	}
	inline void	unlock()
	{
		LeaveCriticalSection( &m_csLock );
	}
private:
	CRITICAL_SECTION m_csLock;
};

/*
*	自动锁，利用局部变量的声明周期，自动调用解锁功能
*/
class AutoLock : public NoCopyable
{
public:
	AutoLock(Mutex* pMutex)
		:m_pMutex(pMutex),
		m_bLocked(false)
	{
		this->lock();
	}

	~AutoLock()
	{
		this->unlock();
	}

	inline void lock()
	{
		if(m_pMutex && !m_bLocked)
		{
			m_pMutex->lock();
			m_bLocked  = true;
		}
	}

	inline void unlock()
	{
		if(m_pMutex && m_bLocked)
		{
			m_pMutex->unlock();
			m_bLocked = false;
		}
	}
private:
	Mutex*	m_pMutex;
	bool	m_bLocked;
};
#endif//__2014_03_02_CMUTEX_H__