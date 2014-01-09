#include "Acceptor.h"
#include "ClientSocket.h"
//////////////////////////////////////////////////////////////////////////
// SocketPool
SocketPool::SocketPool()
	:Thread("SocketPool HeartCheck"), 
	m_nHeartCheckGap(S_DEFAULT_HEARTGAP)
{

}
SocketPool::~SocketPool()
{
	final();
	join();
}
void SocketPool::clear()
{
	checkQueue();
	ClientTaskContainer::iterator it = m_connectSockets.begin();
	while( it != m_connectSockets.end() )
	{
		(*it)->release();
		it++;
	}
	m_connectSockets.clear();
}

bool	SocketPool::run()
{
	while(!isFinal())
	{
		checkQueue();

		ClientTaskContainer::iterator it = m_connectSockets.begin();
		time_t CurTime = time(NULL);
		while( it != m_connectSockets.end() )
		{
			ClientSocket* pClient = *it;
			time_t LastHeart = pClient->getLastHeart();

			if( pClient->isFinal() //连接已经失效
				|| CurTime - LastHeart > m_nHeartCheckGap) //连接心跳超时
			{
				it = m_connectSockets.erase(it);
				pClient->release();
				continue;
			}
			it++;
		}
	}
	clear();// 线程结束了清除连接池中的数据
	return true;
}
//////////////////////////////////////////////////////////////////////////
//AcceptSocket
bool AcceptSocket::startServer(int nPort)
{
	//如果已经启动过服务则直接返回
	if(m_bIsServering)
	{
		Logger::getInstace()->warn("AcceptSocket[ %d ] 重复启动服务.", m_hSocket);
		return false;
	}
	m_bIsServering = true;

	//启动监听服务，bind() 端口 且 listen()
	if( !TCPSocket::startServer(nPort) )
	{
		Logger::getInstace()->error("AcceptSocket[ %d ] 绑定端口[ %d ] 失败.", m_hSocket, nPort);
		return false;
	}
	//线程池心跳线程启动
	if( !m_SocketPool.start() )
	{
		Logger::getInstace()->error("AcceptSocket[ %d ] 连接池心跳线程启动失败.", m_hSocket);
		return false;
	}
	//将套接字绑定到完成端口上，并将AcceptSocket对象指针作为绑定的Key
	if( !IOCPDriver::getInstance()->bindIocpPort(this) )
	{
		Logger::getInstace()->error("AcceptSocket[ %d ] 绑定到完成端口失败", m_hSocket);
		return false;
	}
	//投递一定数量的Accept操作, 默认为10个accpet
	for(int i = 0; i < s_maxAcceptCon; i++)
	{
		if( !addAccept(&m_olpAccept[i]) )
		{
			Logger::getInstace()->error("AcceptSocket[ %d ] addAccept[ %d ] 失败", m_hSocket, i);
			return false;
		}
	}

	return true;
}

bool AcceptSocket::addAccept(LPAcceptOverlappedData pOvoerlappedData)
{
	if(m_hSocket == INVALID_SOCKET) 
	{
		Logger::getInstace()->warn("AcceptSocket >> addAcept >> 出现INVALID_SOCKET");
		return false;
	}
	*pOvoerlappedData = AcceptOverlappedData();

	//创建一个套接字用于准备接受连接
	TCPSocket acceptSocket;
	acceptSocket.init();

	SOCKET hSockClient = acceptSocket.dettach();
	pOvoerlappedData->m_hAccept = hSockClient;

	//投递接受连接操作
	if ( !::AcceptEx( m_hSocket,
		hSockClient,
		pOvoerlappedData->m_outPutBuf, 0,
		pOvoerlappedData->m_dwLocalAddressLength, 
		pOvoerlappedData->m_dwRemoteAddressLength, 
		NULL, &pOvoerlappedData->Overlapped ) )
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			Logger::getInstace()->error("AcceptSocket[ %d ] 投递AcceptEx 失败", m_hSocket);
			return false;
		}
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
//IOWorkThread
bool IOWorkThread::run()
{
	while(!isFinal())
	{
		checkQueue();
		if(m_workTaskList.size() <= 0)
		{
			msleep(10);
			continue;
		}
		
		while( !m_workTaskList.empty() )
		{
			IOTask Task = m_workTaskList.back();
			m_workTaskList.pop_back();
			Task.exec();
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//IOWorkFactory
IOWorkFactory::IOWorkFactory()
	:m_TaskSize (0),
	m_bIsStart(false)
{
	
}

IOWorkFactory::~IOWorkFactory()
{
	for(DWORD i = 0; i < m_workThreadNum; i++)
	{
		IOWorkThread* pThread = m_ThreadList.back();
		m_ThreadList.pop_back();
		pThread->final();
		pThread->join();
		delete pThread;
	}
}

bool IOWorkFactory::AddTask(const IOTask& Task)
{
	m_mutex.lock();
	m_TaskSize++;
	int nIndex = m_TaskSize%m_workThreadNum;
	m_mutex.unlock();
	//按每个工作线程轮流放一个
	m_ThreadList[nIndex]->addTask(Task);

	return true;
}
void IOWorkFactory::start()
{
	if(m_bIsStart) return ;
	m_bIsStart = true;
	for(DWORD i = 0; i < m_workThreadNum; i++)
	{
		IOWorkThread* pThread = new IOWorkThread();
		pThread->start();
		m_ThreadList.push_back(pThread);
	}
}