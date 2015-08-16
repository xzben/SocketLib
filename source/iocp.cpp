#include "iocp.h"
#include "GlobalController.h"
#include "socket.h"
#include "SocketDriver.h"

#if (CUR_PLATFROM == PLATFORM_WIN32)

class   OverlappedData
{
public:
	OVERLAPPED Overlapped;
	SocketIOType OperationType;

	static void free(OverlappedData*& lp)
	{
		SAFE_DELETE(lp);
	}

	OverlappedData(SocketIOType type)
	{
		memset(&Overlapped, 0, sizeof(OVERLAPPED));
		OperationType = type;
	}
};

class AcceptOverlappedData : public OverlappedData
{
public:
	static void free(AcceptOverlappedData*& lp)
	{
		SAFE_DELETE(lp);
	}
	static AcceptOverlappedData* create()
	{
		return new AcceptOverlappedData;
	}

	AcceptOverlappedData()
	:OverlappedData(IO_Accept)
	{
		m_dwLocalAddressLength = sizeof(SOCKADDR_IN)+16;//客户端局域网IP
		m_dwRemoteAddressLength = sizeof(SOCKADDR_IN)+16;//客户端外网IP
		memset(m_outPutBuf, 0, sizeof(SOCKADDR_IN)* 2 + 32);
	}
	/**
	* 指向存有连接进来的客户端局域网和外网地址的内存
	* 必须使用动态分配的内存块
	* 传递给AcceptEx()的第3个参数
	*
	*/
	char m_outPutBuf[sizeof(SOCKADDR_IN)* 2 + 32];
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
};

class RWOverlappedData : public OverlappedData
{
public:
	static void free(RWOverlappedData*& lp)
	{
		SAFE_DELETE(lp);
	}
	static RWOverlappedData* create(SocketIOType	type)
	{
		return new RWOverlappedData(type);
	}

	RWOverlappedData(SocketIOType	type)
		:OverlappedData(type)
	{
		_wsaBuffer.buf = NULL;
		_wsaBuffer.len = 0;
		package = nullptr;
	}

	WSABUF _wsaBuffer;	//WSARecv接收缓冲数据,传递给WSARecv()的第2个参数
	Package* package;
};

class ConnectOverlappedData : public OverlappedData
{
public:
	static void free(ConnectOverlappedData*& lp)
	{
		SAFE_DELETE(lp);
	}
	static ConnectOverlappedData* create(SERVER_HANDLE handle)
	{
		return new ConnectOverlappedData(handle);
	}

	ConnectOverlappedData(SERVER_HANDLE handle)
		: OverlappedData(IO_Connect)
		, requestServer(handle)
	{

	}
	
	SERVER_HANDLE  requestServer;
};

typedef OverlappedData*			LPOverlappedData;
typedef AcceptOverlappedData*	LPAcceptOverlappedData;
typedef ConnectOverlappedData*	LPConnectOverlappedData;
typedef RWOverlappedData*		LPRWOverlappedData;
//////////////////////////////////////////////////////////////////////////
IOCPDriver::IOCPDriver()
{
	//创建完成端口，并制定最大并行线程数为cpu内核数
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (NULL == m_hCompletionPort)
	{
		LOG_FATAL("IOCPDriver 创建完成端口失败");
	}
}

IOCPDriver::~IOCPDriver()
{
	if (m_hCompletionPort != NULL)
		CloseHandle(m_hCompletionPort);
}

int32_t IOCPDriver::poll_event_process(IOEvent *events, int32_t max, int waittime /*=-1*/)
{
	if (max <= 0) 
		return 0;

	DWORD   dwNumRead = 0; // 记录当前 IO 事件的通信数据 size
	LPOverlappedData lpOverlapped = NULL; // 获取事件对应的 Overlapped 

	dwNumRead = 0;
	SOCKET_HANDLE sock = INVALID_SOCKET;

	IOEvent event;
	do 
	{
		if (!GetQueuedCompletionStatus(m_hCompletionPort, &dwNumRead, (LPDWORD)&sock, (LPOVERLAPPED*)&lpOverlapped, waittime))
		{
			DWORD dwErrorCode = ::GetLastError();
			//////////////////////////////////////////////////////////////////////////
			//合法的错误
			if (WAIT_TIMEOUT == dwErrorCode ||  // 超时
				ERROR_INVALID_HANDLE == dwErrorCode) //启动时，未知原因非法句柄，可能是捕获到前一次程序关闭后未处理的完成事件
			{
				return 0;
			}
			//////////////////////////////////////////////////////////////////////////
			//不正常的error
			LOG_DEBUG("IOCPDriver >> GetQueuedCompletionStatus failed >> error [%d] socket[%d] ", dwErrorCode, sock);
			if (ERROR_OPERATION_ABORTED == dwErrorCode) //Accept用于接收新连接的socket关闭
			{
				LOG_FATAL("IOCPDriver >> GetQueuedCompletionStatus >> Accept 用于接收新连接的socket关闭");
				OverlappedData::free(lpOverlapped);
				accept(sock);

				return 0;
			}
			if (IO_Accept == lpOverlapped->OperationType) //Accept上的socket关闭，重新投递监听
			{
				LOG_ERROR("IOCPDriver >> GetQueuedCompletionStatus 失败 >> Accept上的socket关闭，重新投递监听");
				OverlappedData::free(lpOverlapped);
				accept(sock);
			}
			else if(IO_Connect == lpOverlapped->OperationType)
			{
				LOG_DEBUG("IOCPDriver >> GetQueuedCompletionStatus 链接失败");
				OverlappedData::free(lpOverlapped);
				event.evt_type = IO_Error;
				Socket connectSock(sock);
				connectSock.close();
			}
			else//客户端异常断开，拔网线，断电，终止进程
			{
				LOG_DEBUG("IOCPDriver >> GetQueuedCompletionStatus failed 监听的 ClientSocket [%d] 意外关闭", sock);
				OverlappedData::free(lpOverlapped);
				event.evt_type = IO_Close;
			}
		}
		else if (sock == INVALID_SOCKET)	//"Iocp返回空指针！"
		{
			LOG_FATAL("IOCPDriver 事件队列出队列时出现空指针");
			return 0;
		}
		else if (0 == dwNumRead && IO_Read == lpOverlapped->OperationType) //连接断开
		{
			OverlappedData::free(lpOverlapped);
			event.evt_type = IO_Close;
		}
		else
		{
			event.evt_type = lpOverlapped->OperationType;
			if (IO_Accept == lpOverlapped->OperationType)
			{
				accept(sock);
				LPAcceptOverlappedData acpOvelapped = (LPAcceptOverlappedData)(lpOverlapped);
				Socket sock;
				sock.attach(acpOvelapped->m_hAccept);
				sock.updateAcceptContext();
				event.acpt_sock = sock.dettach();
			}
			else if (IO_Read == lpOverlapped->OperationType)
			{
				LPRWOverlappedData rwOverladed = LPRWOverlappedData(lpOverlapped);
				Package* package = rwOverladed->package;
				PackageSize readSize = (PackageSize)dwNumRead;
				package->FillData(readSize);
				if (!package->isFillComplete())
				{
					event.evt_type = IO_ReadPart;
					recv(sock, package);
				}
				else
				{
					event.package = package;
					Package* newPackage = CSocketDriver::getInstance()->resetSocketPackage(sock);
					ASSERT(newPackage != package);
					recv(sock, newPackage);
				}
			}
			else if (IO_Write == lpOverlapped->OperationType)
			{
				LPRWOverlappedData rwOverladed = LPRWOverlappedData(lpOverlapped);
				Package* package = rwOverladed->package;
				PackageSize sendSize = (PackageSize)dwNumRead;
				package->offsetData(sendSize);
				send(sock, package);
			}
			else if (IO_Connect == lpOverlapped->OperationType)
			{
				LPConnectOverlappedData conOvelapped = (LPConnectOverlappedData)(lpOverlapped);
				event.evt_type = IO_Connect;
				event.connect_server = conOvelapped->requestServer;
				Socket conSock(sock);
				conSock.updateConnectContext();
				conSock.dettach();
			}
			OverlappedData::free(lpOverlapped);
		}
	} while (0);

	event.evt_sock = sock;
	events[0] = event;

	return 1;
}

int32_t IOCPDriver::poll_add(SOCKET_HANDLE sock)
{
	if (INVALID_SOCKET == sock)
	{
		LOG_ERROR("IOCPDriver >> poll_add 尝试绑定无效套接字");
		return -1;
	}
	if (NULL == ::CreateIoCompletionPort((HANDLE)sock, m_hCompletionPort, (ULONG_PTR)sock, 0))
	{
		LOG_ERROR("socket [%d] 绑定到完成端口失败", sock);
		return -1;
	}

	return 0;
}

int32_t IOCPDriver::poll_del(SOCKET_HANDLE sock)
{
	return 0;
}

int32_t IOCPDriver::poll_listen(SOCKET_HANDLE sock)
{
	if (poll_add(sock) != 0)
		return -1;

	for (int i = 0; i < 10; i++)
	{
		accept(sock);
	}

	return 0;
}

int32_t IOCPDriver::poll_connect(SERVER_HANDLE handle, const short port, const char* ip)
{
	Socket socket(Socket::SOCK_TCP);
	socket.setBlocked(false);
	socket.bind(InterAddress(0));

	//重置overlapped
	LPConnectOverlappedData overlapConnect = ConnectOverlappedData::create(handle);
	if (overlapConnect == nullptr) return -1;
	
	InterAddress addr(port, ip);

	DWORD sendCnt;
	SOCKET_HANDLE sock = socket.getHandle();
	poll_add(sock);
	
	
	LPFN_CONNECTEX pConnectEx;

	DWORD dwBytes = 0;
	GUID funcGuide = WSAID_CONNECTEX;
	if (0 != ::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&funcGuide, sizeof(funcGuide), &pConnectEx,
		sizeof(pConnectEx), &dwBytes, NULL, NULL))
	{
		return -1;
	}

	socket.dettach();
	//投递数据接收操作
	if (!pConnectEx(sock,
		addr.getAddress(), addr.getAddrLen(), NULL, 0,
		&sendCnt, &overlapConnect->Overlapped))
	{
		int nErrCode = WSAGetLastError();
		if (ERROR_IO_PENDING != nErrCode)
		{
			ConnectOverlappedData::free(overlapConnect);
			LOG_ERROR("Connected [ %d | %s ] failed !", port, ip);
			return -1;
		}
	}

	return 0;
}

int32_t IOCPDriver::poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz)
{
	Package *package = Package::create(sz, buf);
	if (package->isFillComplete())
		send(sock, package);
	else
	{
		Package::free(package);
		return -1;
	}

	return 0;
}

int32_t IOCPDriver::poll_recv(SOCKET_HANDLE sock)
{
	return recv(sock);
}

//////////////////////////////////////////////////////////////////////////
int32_t		IOCPDriver::recv(SOCKET_HANDLE sock, Package* pack /*= nullptr*/)
{
	//重置overlapped
	LPRWOverlappedData overlapRead = RWOverlappedData::create(IO_Read);
	Package *package = pack == nullptr ? CSocketDriver::getInstance()->getSocketPackage(sock) : pack;
	if (package == nullptr)
		return -1;

	void *pDataBuffer = nullptr;
	PackageSize nDataSize = 0;
	overlapRead->package = package;
	package->getFillData(nDataSize, pDataBuffer);
	overlapRead->_wsaBuffer.buf = (char*)pDataBuffer;
	overlapRead->_wsaBuffer.len = nDataSize;

	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	//投递数据接收操作
	if (SOCKET_ERROR == ::WSARecv(sock,
		&overlapRead->_wsaBuffer,
		1, &dwRecv, &dwFlags,
		&overlapRead->Overlapped, NULL))
	{
		int nErrCode = WSAGetLastError();
		if (ERROR_IO_PENDING != nErrCode)
		{
			RWOverlappedData::free(overlapRead);
			LOG_ERROR("ClientSocket[%d], addRecv 失败", sock);
			return -1;
		}
	}
	
	return 0;
}

int32_t		IOCPDriver::send(SOCKET_HANDLE sock, Package* package)
{
	//此包已经完整发送
	if (package->isSendComplete())
	{
		Package::free(package);
		return 0;
	}

	//投递新的发送事件
	LPRWOverlappedData overlapWrite = RWOverlappedData::create(IO_Read);

	void *pDataBuffer = nullptr;
	PackageSize nDataSize = 0;
	package->getSendData(nDataSize, pDataBuffer);

	overlapWrite->package = package;
	overlapWrite->_wsaBuffer.buf = (char*)pDataBuffer;
	overlapWrite->_wsaBuffer.len = nDataSize;

	DWORD dwSend = 0;
	DWORD dwFlags = 0;

	//投递数据接收操作
	if (SOCKET_ERROR == ::WSASend(sock,
		&overlapWrite->_wsaBuffer,
		1, &dwSend, dwFlags,
		&overlapWrite->Overlapped, NULL))
	{
		int nErrCode = WSAGetLastError();
		if (ERROR_IO_PENDING != nErrCode)
		{
			LOG_ERROR("ClientSocket[%d], addSend 失败", sock);
			RWOverlappedData::free(overlapWrite);
			return -1;
		}
	}
	return 0;
}

int32_t		IOCPDriver::accept(SOCKET_HANDLE sock)
{
	if (sock == INVALID_SOCKET)
	{
		LOG_WARN("AcceptSocket >> addAcept >> 出现INVALID_SOCKET");
		return -1;
	}
	LPAcceptOverlappedData overlappedData = AcceptOverlappedData::create();

	//创建一个套接字用于准备接受连接
	Socket socket;
	socket.open(Socket::SOCK_TCP);
	socket.setBlocked(false);
	SOCKET_HANDLE hSockClient = socket.dettach();
	overlappedData->m_hAccept = hSockClient;

	//投递接受连接操作
	if (!::AcceptEx(sock,
		hSockClient,
		overlappedData->m_outPutBuf, 0,
		overlappedData->m_dwLocalAddressLength,
		overlappedData->m_dwRemoteAddressLength,
		NULL, &overlappedData->Overlapped))
	{
		int nErrCode = WSAGetLastError();
		if (ERROR_IO_PENDING != nErrCode)
		{
			LOG_ERROR("AcceptSocket[ %d ] 投递AcceptEx 失败", sock);
			AcceptOverlappedData::free(overlappedData);
			return -1;
		}
	}

	return 0;
}

#endif // (CUR_PLATFROM == PLATFORM_WIN32)