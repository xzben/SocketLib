#ifndef __2014_03_02_CThread_H__
#define __2014_03_02_CThread_H__

#include "CObject.h"
#include "Mutex.h"
/*
*	线程封装
*/
class Thread : public NoCopyable
{
public:
	Thread(std::string strName, bool bJoinable = true)
		:m_strThreadName(strName)
		,m_bComplete(false)
		,m_bActive(false)
		,m_hThreadHandle(NULL)
		,m_bJoinable(bJoinable)
	{
		
	}
	virtual ~Thread()
	{
		if(NULL != m_hThreadHandle)
		{
			::CloseHandle(m_hThreadHandle);
		}
	}
	//线程睡眠 单位为秒
	void	sleep(const long sec)	const	{ ::Sleep( sec*1000 );	}
	//线程睡眠 单位为毫秒
	void	msleep(const long msec)	const	{ ::Sleep( msec );		}
	//线程睡眠，单位为微妙
	void	usleep(const long usec) const	{ ::Sleep( usec/1000 ); }
	//判断线程是否已经激活
	bool	isActive()				const	{ return m_bActive;		}
	//判断是否要求过结束线程
	bool	isFinal()				const	{ return m_bComplete;	}
	//请求结束线程
	void	final()							{ m_bComplete = true;	}
	//线程是否可Join, join able 的线程线程结束时，不会删除线程对象，否则在线程结束时会自动delete线程对象。 
	bool	isJoinable()			const	{ return m_bJoinable;	}
	//线程启动
	bool	start();
	//等价线程结束，与调用线程合并
	void	join();
	//线程执行内容的回调函数
	virtual bool run() = 0;
	//获取线程的名字
	const std::string&	getThreadName()const{ return m_strThreadName; }
protected:
	static DWORD __stdcall ThreadFunc(void *arg);
protected:
	std::string			m_strThreadName; // 线程名
	Mutex				m_ThreadMutex;	 // 线程互斥锁
	volatile bool		m_bComplete;	 // 线程结束开关	
	volatile bool		m_bActive;		 // 线程运行标志
	HANDLE				m_hThreadHandle; // 线程句柄
	// 线程是否需要合并，joinable 的线程在线程结束的时候会将对象delete，否则对象在线程结束了之后还没有delete，任然可以继续使用.
	bool				m_bJoinable;
};

#endif//__2014_03_02_CThread_H__
