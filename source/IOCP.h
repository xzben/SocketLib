#ifndef __2014_01_04_IOCP_H__
#define __2014_01_04_IOCP_H__

#include "svrEngine.h"
#include <vector>
#include <queue>


class ClientSocket;

typedef   enum   enum_IOOperationType   
{     
	IO_Write,     // 写
	IO_Read,	  // 读
	IO_Accept,	  // 接收连接
}IOOperationType;

typedef   struct   st_OverlappedData   
{   
	OVERLAPPED Overlapped;
	IOOperationType OperationType;

	st_OverlappedData( enum_IOOperationType type )
	{
		ZeroMemory( &Overlapped, sizeof(OVERLAPPED) );
		OperationType = type;
	}
}OverlappedData,   *LPOverlappedData;

//IOCPDriver 的 事件监视线程
class IocpRecvThread : public Thread
{
public:
	IocpRecvThread():Thread("zIocpRecvThread"){};
	~IocpRecvThread(){};

	virtual bool run()override;
};

class IOCPDriver : public NoCopyable
{
public:
	// IOCPDriver 开启服务
	void start();
	// 获取 完成端口句柄
	HANDLE	getCompletionPortHandle() { return m_hCompletionPort; }
	//绑定一个 Socket 套接字到完成端口 并将它所在对象 TCPSocket 的指针作为关联 Key 一起绑定
	bool bindIocpPort(TCPSocket *pSocket);// 绑定IO端口
	// 等待IOCPDriver IO事件线程结束
	void wait_event_loop();
	// 通知所有 IOCPDriver 的IO 事件线程结束
	void end_event_loop();

	// 此处选择最佳 IOCP 轮询线程为 CPU 的数目
	short IOCPDriver::getBestThreadNumber()
	{
		SYSTEM_INFO sysInfo;
		::GetSystemInfo(&sysInfo);
		int nCpuNum = sysInfo.dwNumberOfProcessors;

		return nCpuNum;
	}
	// IOCPDriver 的单例对象获取
	static IOCPDriver* getInstance()
	{
		static IOCPDriver Instace;
		static bool		bIsStart = false;
		if( !bIsStart )
		{
			Instace.start();
			bIsStart = true;
		}

		return &Instace;
	}
protected:
	IOCPDriver();
	~IOCPDriver();
private:
	HANDLE	m_hCompletionPort;
	DWORD	m_dwThreadCount;
	typedef	std::vector<IocpRecvThread*>	IOThreadContainer;
	IOThreadContainer						m_ThreadList;
	bool									m_bIsStart;
};

#endif//__2014_01_04_IOCP_H__