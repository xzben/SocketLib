/********************************************************************************
*	文件名称:	Socket.cpp														*
*	创建时间：	2014/03/18														*
*	作   者 :	xzben															*
*	文件功能:	本文件主要实现一个和平台无关的Socket类								*
*********************************************************************************/

#include "socket.h"
#include "socket_option.h"
#include "socket_handle.h"
#include "common.h"

Socket::Socket()
{
	m_hSocket = INVALID_SOCKET_HANDLE;
}

Socket::Socket(SOCKET_HANDLE hSocket)
{
	attach(hSocket);
}

Socket::Socket(const SockType type, const int32_t nAf /*= AF_INET*/, const int32_t nProtocl /*= 0*/)
{
	open(type, nAf, nProtocl);
}

Socket::~Socket()
{
	close();
}
 
void	Socket::attach(SOCKET_HANDLE hSocket)
{
	ASSERT(INVALID_SOCKET_HANDLE != hSocket);
	close();
	m_hSocket = hSocket;
}
  
SOCKET_HANDLE	Socket::dettach()
{
	SOCKET_HANDLE retHandle = m_hSocket;
	m_hSocket = INVALID_SOCKET_HANDLE;
	return retHandle;
}
  
bool	Socket::isValidTcp()
{
	if (INVALID_SOCKET_HANDLE == m_hSocket)
		return false;

	SocketOption option;
	return SOCK_TCP == option.getSocketType(*this);
}

SOCKET_HANDLE	Socket::getHandle()
{
	return m_hSocket;
}
  
bool	Socket::open(const SockType type, const int32_t nAf /*= AF_INET*/, const int32_t nProtocl /*= 0*/)
{
	ASSERT(INVALID_SOCKET_HANDLE == m_hSocket);
  
	m_hSocket = ::socket(nAf, type, nProtocl);
  
	ASSERT(INVALID_SOCKET_HANDLE != m_hSocket);
  
	return (INVALID_SOCKET_HANDLE != m_hSocket);
  
}
  
bool	Socket::shutdown(int32_t how /*= SD_BOTH*/)
{
	if (INVALID_SOCKET_HANDLE == m_hSocket)	
  		return true;
  
	return (0 == ::shutdown(m_hSocket, how));
}
  
bool	Socket::close()
{
	if (INVALID_SOCKET_HANDLE == m_hSocket) 
  		return true;

	bool bRet = false;
#if (CUR_PLATFROM == PLATFORM_WIN32)
	bRet = (0 == ::closesocket(m_hSocket));
#else
	bRet = (0 == ::close(m_hSocket));
#endif//_WIN32
	m_hSocket = INVALID_SOCKET_HANDLE;

	return bRet;
}
  
bool	Socket::bind(const InterAddress& addrBind)
{
	if (SOCKET_ERROR == ::bind(m_hSocket, addrBind.getAddress(), addrBind.getAddrLen()))
	{
  		int32_t nErr = ::GetLastError();
  		//常见的err 为 WSAEADDRINUSE  10048
  		return false;
	}
	return true;
}
  
bool	Socket::listen(int32_t nBacklog /*= 10*/)
{
	if (SOCKET_ERROR == ::listen(m_hSocket, nBacklog))
	{
  		int32_t nErr = ::GetLastError();
  		return false;
	}
	return true;
}

bool Socket::accept(Socket& sockCon, InterAddress* remoteAddr /*= nullptr*/)
{
	InterAddress addrCon;
	socklen_t nAddrLen = addrCon.getAddrLen();
  
	SOCKET_HANDLE hConnSocket = ::accept(m_hSocket, addrCon.getAddress(), &nAddrLen);
	if ( INVALID_SOCKET_HANDLE == hConnSocket)
	{
  		return false;
	}
  
	sockCon.attach(hConnSocket);
	if (nullptr != remoteAddr)
	{
		remoteAddr->open(addrCon.getAddress());
	}
	return true;
}

bool Socket::accept(const TimeValue& tmVal, Socket& sockCon, InterAddress* remoteAddr /* = nullptr */)
{
	if (!isReadReady(tmVal))
		return false;

	return accept(sockCon, remoteAddr);
}

bool	Socket::connect(const InterAddress& addrCon)
{
	if (SOCKET_ERROR == ::connect(m_hSocket, addrCon.getAddress(), addrCon.getAddrLen()))
	{
		int32_t nErr = ::GetLastError();
		return false;
	}

	return true;
}

bool	Socket::connect(const InterAddress& addrCon, const TimeValue& tmVal)
{
	SocketOption option;
	//为了实现连接超时的功能，先将socket设置为非阻塞模式，然后再恢复回原先的模式
	bool bIsBlock = option.isBlockMode(*this);
	if ( bIsBlock )  option.setBlockMode(*this, false);

	int32_t nRet = ::connect(m_hSocket, addrCon.getAddress(), addrCon.getAddrLen());

	if (nRet < 0) 
	{
		int32_t error = ::GetLastError();
#if (CUR_PLATFROM == PLATFORM_WIN32)
		if ( WSAEWOULDBLOCK == error)
#else
		if (EINPROGRESS == error)
#endif
		{
			fd_set wset;
			FD_ZERO(&wset);
			FD_SET(m_hSocket, &wset);
#if (CUR_PLATFROM == PLATFORM_WIN32)
			int32_t nWidth = 0;
#else
			int32_t nWidth = m_hSocket + 1;
#endif
			if (::select(nWidth, NULL, &wset, NULL, tmVal.getTimeval()) > 0 && FD_ISSET(m_hSocket, &wset))
			{
				int32_t valopt;
				socklen_t nLen = sizeof(valopt);
				
				getsockopt(m_hSocket, SOL_SOCKET, SO_ERROR, (char*)(&valopt), &nLen);
				if (valopt) 
				{
					//fprintf(stderr, "Error in connection() %d - %s/n", valopt, strerror(valopt));
					//exit(0);
					return false;
				}
			}
			else
			{
				//fprintf(stderr, "Timeout or error() %d - %s/n", valopt, strerror(valopt));
				//exit(0);
				return false;
			}
		}
		else
		{
			//fprintf(stderr, "Error connecting %d - %s/n", errno, strerror(errno));
			//exit(0);
			return false;
		}
	}
	
	//为了实现连接超时的功能，先将socket设置为非阻塞模式，然后再恢复回原先的模式
	if(bIsBlock) option.setBlockMode(*this, true);

	return true;
}

int32_t	Socket::getReadyStatus(const TimeValue& tmVal, bool *pReadReady /*= nullptr*/,
	bool* pWriteReady /*= nullptr*/, bool* pExceptReady/*= nullptr*/)
{
	if (INVALID_SOCKET_HANDLE == m_hSocket)
		return -1;

	fd_set setRead, setWrite, setExcept;
	FD_ZERO(&setRead); FD_ZERO(&setWrite); FD_ZERO(&setExcept);
	FD_SET(m_hSocket, &setRead); FD_SET(m_hSocket, &setWrite); FD_SET(m_hSocket, &setExcept);

	SET_PTR_VALUE_SAFE(pReadReady, false);
	SET_PTR_VALUE_SAFE(pWriteReady, false);
	SET_PTR_VALUE_SAFE(pExceptReady, false);

#if (CUR_PLATFROM == PLATFORM_WIN32)
	int32_t selectWith = 0;
#else
	int32_t selectWith = m_hSocket + 1;
#endif

	int32_t nRet = ::select(selectWith, 
		pReadReady	 ? &setRead  : nullptr,
		pWriteReady	 ? &setWrite : nullptr,
		pExceptReady ? &setExcept: nullptr, tmVal.getTimeval());


	if (nRet > 0)
	{
		if (FD_ISSET(m_hSocket, &setRead))	 SET_PTR_VALUE_SAFE(pReadReady, true);
		if (FD_ISSET(m_hSocket, &setWrite)) SET_PTR_VALUE_SAFE(pWriteReady, true);
		if (FD_ISSET(m_hSocket, &setExcept))SET_PTR_VALUE_SAFE(pExceptReady, true);
	}

	return nRet;
}

bool Socket::isReadReady(const TimeValue& tmVal)
{
	bool bReady = false;
	VERIFY(getReadyStatus(tmVal, &bReady, nullptr, nullptr) >= 0);

	return bReady;
}


bool Socket::isWriteReady(const TimeValue& tmVal)
{
	bool bReady = false;
	VERIFY(getReadyStatus(tmVal, nullptr, &bReady, nullptr) >= 0);

	return bReady;
}

int32_t	Socket::recv(void* pBuf, int32_t nLen, int32_t nFlag /*= 0*/)
{
	int32_t nRecvSize = ::recv(m_hSocket, (char*)pBuf, nLen, nFlag);
	if (SOCKET_ERROR == nRecvSize)
	{
		int32_t nErr = ::GetLastError();
	}

	return nRecvSize;
}

int32_t	Socket::recv(void* pBuf, int32_t nLen, const TimeValue& tmVal, int32_t nFlag /* = 0 */)
{
	if (!isReadReady(tmVal))
		return 0;

	return recv(pBuf, nLen, nFlag);
}

int32_t	Socket::send(const void *pBuf, const int32_t nLen, int32_t nFlag /*= 0*/)
{
	int32_t nSendSize = ::send(m_hSocket, (char*)pBuf, nLen, nFlag);
	if (SOCKET_ERROR == nSendSize)
	{
		int32_t nErr = ::GetLastError();
	}

	return nSendSize;
}

int32_t Socket::send(const void *pBuf, const int32_t nLen, const TimeValue& tmVal, int32_t nFlag /* = 0 */)
{
	if (!isWriteReady(tmVal))
		return 0;

	return send(pBuf, nLen, nFlag);
}


int32_t Socket::recvfrom(void* pBuf, int32_t nLen, InterAddress& addFrom, int32_t nFlag /*=0*/)
{
	socklen_t nFrom = addFrom.getAddrLen();

	int32_t nRecvSize = ::recvfrom(m_hSocket, (char*)pBuf, nLen, nFlag, addFrom.getAddress(), &nFrom);
	if (SOCKET_ERROR == nRecvSize)
	{
		int32_t nErr = ::GetLastError();
	}

	return nRecvSize;
	
}

int32_t Socket::recvfrom(void* pBuf, int32_t nLen, InterAddress& addFrom, const TimeValue& tmVal, int32_t nFlag /* = 0 */)
{
	if (!isReadReady(tmVal))
		return 0;

	return recvfrom(pBuf, nLen, addFrom, nFlag);
}

int32_t Socket::sendto(const void *pBuf, const int32_t nLen, const InterAddress& addrTo, int32_t nFlag /*= 0*/)
{
	int32_t nSendSize = ::sendto(m_hSocket, (char*)pBuf, nLen, nFlag, addrTo.getAddress(), addrTo.getAddrLen());
	if (SOCKET_ERROR == nSendSize)
	{
		int32_t nErr = ::GetLastError();
	}

	return nSendSize;
}

int32_t Socket::sendto(const void *pBuf, const int32_t nLen, const InterAddress& addrTo, const TimeValue& tmVal, int32_t nFlag /* = 0 */)
{
	if (!isWriteReady(tmVal))
		return 0;

	return sendto(pBuf, nLen, addrTo, nFlag);
}

bool Socket::setBlocked(bool bIsBlock)
{
#if (CUR_PLATFROM == PLATFORM_WIN32)
	m_bIsBlock = bIsBlock;
#endif

	SOCKET_HANDLE hSock = m_hSocket;
	if (INVALID_SOCKET_HANDLE == hSock)
		return false;

#if (CUR_PLATFROM == PLATFORM_WIN32)
	u_long iMode = bIsBlock ? 0 : 1;
	return (SOCKET_ERROR != ioctlsocket(hSock, FIONBIO, &iMode));

#elif (CUR_PLATFROM == PLATFORM_UNKNOW)

	int32_t flag;
	if (flag = fcntl(hSock, F_GETFL, 0) < 0)
		return false;

	SET_DEL_BIT(flag, O_NONBLOCK, !bIsBlock);

	if (fcntl(hSock, F_SETFL, flag) < 0)
		return false;

	return true;
#endif
}

bool Socket::isBlocked()
{
	if (INVALID_SOCKET_HANDLE == m_hSocket)
		return false;
#if (CUR_PLATFROM == PLATFORM_WIN32)
	return m_bIsBlock;
#elif (CUR_PLATFROM == PLATFORM_UNKNOW)

	int32_t flag;
	if (flag = fcntl(m_hSocket, F_GETFL, 0) < 0)
		return false;

	return QUERY_IS_SET_BIT(flag, O_NONBLOCK);
#endif
}

#if (CUR_PLATFROM == PLATFORM_WIN32)
bool Socket::updateAcceptContext()
{
	return 0 == setsockopt(m_hSocket,
		SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char *)&(m_hSocket), sizeof(m_hSocket));
}

bool Socket::updateConnectContext()
{
	return 0 == setsockopt(m_hSocket, 
		SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,
		NULL, 0);
}
#endif
//////////////////////////////////////////////////////////////////////////
//用于自动装载 SocketLib 用
#if (CUR_PLATFROM == PLATFORM_WIN32)

 class SocketLibLoadHelper
 {
 public:
 	SocketLibLoadHelper()
 	{
 		s_loadSockLib();
 	}
 	~SocketLibLoadHelper()
 	{
 		s_destroySockLib();
 	}
 private:
 	static bool			s_loadSockLib(int32_t nHigh = 2, int32_t nLow = 2);
 	static bool			s_destroySockLib();
 private:
 	static	bool		s_bLoadedSockLib;
 };
 
 bool SocketLibLoadHelper::s_bLoadedSockLib = false;
 bool SocketLibLoadHelper::s_loadSockLib(int32_t nHigh /* = 2 */, int32_t nLow /* = 2 */)
 {
 	if (s_bLoadedSockLib) //已经加载过，直接返回
 		return true;
 	s_bLoadedSockLib = true;
 
 	WORD wVersionRequested;
 	WSADATA wsaData;
 	int32_t err;
 
 	wVersionRequested = MAKEWORD(nHigh, nLow);
 	err = WSAStartup(wVersionRequested, &wsaData);
 	if (err != 0)
 		return false;
 
 	if (LOBYTE(wsaData.wVersion) != nHigh
 		|| HIBYTE(wsaData.wVersion) != nLow)
 	{
 		WSACleanup();
 		return false;
 	}
 	return true;
 }
 bool SocketLibLoadHelper::s_destroySockLib()
 {
 	if (!s_bLoadedSockLib) //还未加载过，或已经卸载了
 		return true;
 
 	s_bLoadedSockLib = false;
 	if (0 != WSACleanup())
 		return false;
 
 	return true;
 }
 
 SocketLibLoadHelper g_socketLibLoadHelper; //定义一个全局变量，使运行环境自动装载和卸载 SocketLib
#endif //_WIN32
//////////////////////////////////////////////////////////////////////////
