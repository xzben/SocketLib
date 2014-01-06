#include "IOCP.h"
#include "Acceptor.h"
#include "ClientSocket.h"

//////////////////////////////////////////////////////////////////////////
// IocpRecvThread
bool IocpRecvThread::run()
{
	DWORD   dwNumRead = 0; // 记录当前 IO 事件的通信数据 size
	LPOverlappedData lpOverlapped = NULL; // 获取事件对应的 Overlapped 
	TCPSocket* pSock = NULL; // 与 IO 事件绑定的 TCPSocket 对象指针

	while( !isFinal() )
	{
		dwNumRead = 0;
		lpOverlapped = NULL;
		pSock = NULL;

		if( !GetQueuedCompletionStatus( IOCPDriver::getInstance()->getCompletionPortHandle(), &dwNumRead, (LPDWORD)&pSock, (LPOVERLAPPED*)&lpOverlapped, INFINITE ))
		{
			DWORD dwErrorCode = ::GetLastError();
			Logger::getInstace()->debug("IOCPDriver >> GetQueuedCompletionStatus 失败 >> error [%d] ", dwErrorCode);

			if ( WAIT_TIMEOUT == dwErrorCode ||  // 超时
				ERROR_INVALID_HANDLE == dwErrorCode ) //启动时，未知原因非法句柄，可能是捕获到前一次程序关闭后未处理的完成事件
			{	
				continue;
			}
			if ( ERROR_OPERATION_ABORTED == dwErrorCode ) //Accept用于接收新连接的socket关闭
			{
				Logger::getInstace()->fatal("IOCPDriver >> GetQueuedCompletionStatus >> Accept用于接收新连接的socket关闭");
				continue;
			}
			if ( IO_Accept == lpOverlapped->OperationType ) //Accept上的socket关闭，重新投递监听
			{
				Logger::getInstace()->error("IOCPDriver >> GetQueuedCompletionStatus 失败 >> Accept上的socket关闭，重新投递监听");
				//((AcceptSocket*)pSock)->addAccept((AcceptSocket::LPAcceptOverlappedData)lpOverlapped);
			}
			else//客户端异常断开，拔网线，断电，终止进程
			{
				Logger::getInstace()->debug("IOCPDriver >> GetQueuedCompletionStatus 失败 >> 监听的 ClientSocket [%d] 意外关闭", pSock->getSocket());
				((ClientSocket*)pSock)->release();
			}
		}
		else if( pSock == NULL )	//"Iocp返回空指针！"
		{
			Logger::getInstace()->fatal("IOCPDriver 事件队列出队列时出现空指针");
			continue;
		}
		else if ( 0 == dwNumRead && IO_Read== lpOverlapped->OperationType ) //连接断开
		{
			((ClientSocket*)pSock)->release();
		}
		else
		{
			if(IO_Accept == lpOverlapped->OperationType)
			{
				((AcceptSocket*)pSock)->onAccept((AcceptSocket::LPAcceptOverlappedData)lpOverlapped);
			}
			else if(IO_Read == lpOverlapped->OperationType)
			{
				((ClientSocket*)pSock)->recvDone(dwNumRead);
			}
			else if(IO_Write == lpOverlapped->OperationType)
			{
				((ClientSocket*)pSock)->sendout(dwNumRead);
			}
		}
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
// IOCPDriver

IOCPDriver::IOCPDriver()
	:m_bIsStart(false)
{
	//创建完成端口，并制定最大并行线程数为cpu内核数
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(NULL == m_hCompletionPort)
	{
		Logger::getInstace()->fatal("IOCPDriver 创建完成端口失败");
	}
}

IOCPDriver::~IOCPDriver()
{
	for( IOThreadContainer::iterator it = m_ThreadList.begin(); 
		it != m_ThreadList.end(); it++)
	{
		(*it)->final(); //通知线程结束
		(*it)->join();  //等待线程结束
		delete (*it);   //删除线程对象
	}
	m_ThreadList.clear();
}

void IOCPDriver::start()
{
	if( m_bIsStart )
	{
		Logger::getInstace()->warn("IOCPDriver 重复启动");
		return;
	}
	m_bIsStart = true;

	m_dwThreadCount = getBestThreadNumber();
	for(DWORD i = 0; i < m_dwThreadCount; i++)
	{
		IocpRecvThread* pThread = new IocpRecvThread();
		m_ThreadList.push_back(pThread);
		pThread->start();
	}
}

bool IOCPDriver::bindIocpPort(TCPSocket *pSocket)// 绑定IO端口
{
	if(NULL == pSocket)
	{
		Logger::getInstace()->error("IOCPDriver >> bindIocpPort 尝试绑定空指针");
		return false;
	}
	if( NULL == ::CreateIoCompletionPort( (HANDLE)pSocket->getSocket(), m_hCompletionPort, (ULONG_PTR)pSocket, m_dwThreadCount ) )
	{
		Logger::getInstace()->error("ClientSocket[%d] 绑定到完成端口失败", pSocket->getSocket());
		return false;
	}
	return true;
}

void IOCPDriver::wait_event_loop()
{
	for( IOThreadContainer::iterator it = m_ThreadList.begin(); 
		it != m_ThreadList.end(); it++)
	{
		(*it)->join();
		delete (*it);
	}
	m_ThreadList.clear();
}

void IOCPDriver::end_event_loop()
{
	for( IOThreadContainer::iterator it = m_ThreadList.begin(); 
		it != m_ThreadList.end(); it++)
	{
		(*it)->final();
	}
}