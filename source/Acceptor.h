#ifndef __2013_01_04_ACCEPTOR_H__
#define __2013_01_04_ACCEPTOR_H__

#include "svrEngine.h"
#include "IOCP.h"
#include "ClientSocket.h"

//连接池，且连接池有心跳功能
class SocketPool : public Thread, public TaskQueue<ClientSocket*>
{
public:
	SocketPool();
	virtual ~SocketPool();

	void	setHeartGap(int nSecGap){	AutoLock lock(&m_SocketPoolMutex); m_nHeartCheckGap = nSecGap; }
	bool	run()override; //心跳线程回调函数
protected:
	void	clear();
	void	_addTask(ClientSocket* pTask) override
	{
		m_connectSockets.push_back(pTask);
	}
private:
	static const int		S_DEFAULT_HEARTGAP = 10;
	int						m_nHeartCheckGap;	// 连接的心跳间隔，默认为 10 秒
	Mutex					m_SocketPoolMutex;	
	typedef	std::vector<ClientSocket*>	ClientTaskContainer;
	ClientTaskContainer		m_connectSockets;	//循环检测容器
};

class AcceptSocket	: public TCPSocket
{
public:
	typedef struct st_AcceptOverlappedData : public st_OverlappedData
	{
		st_AcceptOverlappedData()
			:st_OverlappedData(IO_Accept)
		{
			m_dwLocalAddressLength = sizeof(SOCKADDR_IN) + 16;//客户端局域网IP
			m_dwRemoteAddressLength = sizeof(SOCKADDR_IN) + 16;//客户端外网IP
			memset( m_outPutBuf, 0, sizeof(SOCKADDR_IN)*2+32 );
		}
		/**
		* 指向存有连接进来的客户端局域网和外网地址的内存
		* 必须使用动态分配的内存块
		* 传递给AcceptEx()的第3个参数
		* 
		*/
		char m_outPutBuf[sizeof(SOCKADDR_IN)*2+32];
		/**
		* 客户端局域网IP信息长度
		* 传递给AcceptEx()的第5个参数
		*/
		unsigned long m_dwLocalAddressLength;
		/**
		* 客户端外网IP信息长度
		* 传递给AcceptEx()的第6个参数
		*/
		unsigned long m_dwRemoteAddressLength;
		//新连接对应的套接字
		SOCKET		m_hAccept;
	}AcceptOverlappedData, *LPAcceptOverlappedData;
	friend IocpRecvThread;
public:
	AcceptSocket()
		:m_bIsServering(false)
	{
		if(!init())
		{
			Logger::getInstace()->fatal("Acceptor 初始化失败");
		}
	}
	virtual ~AcceptSocket(){}
	bool startServer(int nPort);
	void setHeartGap(int nSecGap) { m_SocketPool.setHeartGap(nSecGap); };
protected:
	bool addAccept(LPAcceptOverlappedData pOvoerlappedData);
	virtual	bool onAccept(LPAcceptOverlappedData pOvoerlappedData) = 0;
protected:
	static const short		s_maxAcceptCon = 10;		 // 初始投递多少 AcceptEx 操作
	AcceptOverlappedData	m_olpAccept[s_maxAcceptCon]; // 每个 AcceptEx 操作对应的 Overlapped
	bool					m_bIsServering;				 // Accpetor 是否已经开启服务 
	SocketPool				m_SocketPool;				 // Acceptor 得到的连接集合池
};

template<class _ClientType>
class Acceptor : public AcceptSocket
{
protected:
	virtual	bool onAccept(LPAcceptOverlappedData pOvoerlappedData)
	{
		ClientSocket *pAcceptClient = new _ClientType;
		pAcceptClient->attach(pOvoerlappedData->m_hAccept);
		pAcceptClient->updateAcceptContext();
		pAcceptClient->initPeerAddress();
		addAccept(pOvoerlappedData);
		
		//////////////////////////////////////////////////////////////////////////
		//连接打开验证，失败则删除链接
		if( !pAcceptClient->OnOpen() )
		{
			pAcceptClient->disConnect();
			delete pAcceptClient;
			return false;
		}
		if( !IOCPDriver::getInstance()->bindIocpPort(pAcceptClient) )
		{
			assert(false); //关联完成端口失败
			Logger::getInstace()->error("onAccept 中绑定连接到完成端口失败");
			return false;
		}
		
		if( !pAcceptClient->addRecv() ) //投递recv操作
		{
			assert(false);
			Logger::getInstace()->error("onAccept 中给新连接 addRecv() 失败");
			return false;
		}
		m_SocketPool.addTask(pAcceptClient); //加入连接池
		//////////////////////////////////////////////////////////////////////////
		return true;
	}
};

class IOTask
{
public:
	IOTask(ClientSocket* pSocket, IOOperationType IOType, int nSize)
		:m_pSocket(pSocket), m_IOType(IOType), m_IOSize(nSize)
	{
		m_pSocket->addReference();
	}
	bool exec()
	{
		switch(m_IOType)
		{
		case IOOperationType::IO_Read:
			{
				return m_pSocket->OnMsg();
			}
		default:
			assert(false);
		}

		return false;
	}
	IOTask(const IOTask& rTask)
	{
		this->m_pSocket = rTask.m_pSocket;
		this->m_IOType = rTask.m_IOType;
		this->m_IOSize = rTask.m_IOSize;

		this->m_pSocket->addReference(); // 连接对象进入一个 任务对象中，增加引用保证 task 执行完成前对象有效
	}
	~IOTask()
	{
		m_pSocket->decReference(); // task 执行完成 释放任务对象，保持的连接对象的引用
	}
private:
	ClientSocket*	m_pSocket;
	IOOperationType m_IOType;
	int				m_IOSize;
};

class IOWorkThread : public Thread, public TaskQueue<IOTask>
{
public:
	IOWorkThread():Thread("IOWorkThread"){};
	~IOWorkThread(){};
protected:
	virtual bool run()override;
	virtual void	_addTask(IOTask Task)override
	{
		m_workTaskList.push_back(Task);
	}
protected:
	typedef std::vector<IOTask>	TaskContainer;
	TaskContainer	m_workTaskList;
};

class IOWorkFactory : public NoCopyable
{
public:
	static IOWorkFactory* getInstance()
	{
		static IOWorkFactory Instance;
		static bool			 bIsStart = false;
		if( !bIsStart )
		{
			Instance.start();
			bIsStart = true;
		}
		return &Instance;
	}
	
	bool AddTask(const IOTask& Task);
	void start();
protected:
	IOWorkFactory();
	~IOWorkFactory();
protected:
	bool								m_bIsStart;
	static const int					m_workThreadNum = 4;
	typedef	std::vector<IOWorkThread*>	IOThreadContainer;
	IOThreadContainer					m_ThreadList;
	Mutex								m_mutex;
	unsigned int						m_TaskSize;
};
#endif//__2013_01_04_ACCEPTOR_H__