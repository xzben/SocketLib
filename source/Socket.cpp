#include "Socket.h"
#include "Logger.h"

//////////////////////////////////////////////////////////////////////////
//TCPSocket
sockaddr_in TCPSocket::s_getSockAddrIpV4(const char* szIpAddr, const u_short usPort)
{
	sockaddr_in	addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(usPort);
	if(NULL == szIpAddr)
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	else
		addr.sin_addr.S_un.S_addr = inet_addr(szIpAddr);

	return addr;
}
void	TCPSocket::s_getSockAddrIpV4(const sockaddr_in addr, std::string& strIpAddr, u_short &usPort)
{
	usPort = ntohs(addr.sin_port);
	strIpAddr = inet_ntoa(addr.sin_addr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
TCPSocket::TCPSocket()
{
	m_hSocket = INVALID_SOCKET;
	m_bHaveAddr = false;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));
}

bool TCPSocket::init(const int nAf /*= AF_INET*/, const int nProtocl /*= 0*/)
{
	m_hSocket = ::socket(nAf, SOCK_STREAM, nProtocl);
	if(INVALID_SOCKET == m_hSocket)
		return false;
	m_bHaveAddr = false;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));

	return true;
}

TCPSocket::~TCPSocket()
{
	this->close();
}

bool TCPSocket::attach(SOCKET hSocket)
{
	if(INVALID_SOCKET == hSocket)
		return false;

	m_hSocket = hSocket;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));
	m_bHaveAddr = false;
	return true;
}

SOCKET TCPSocket::dettach()
{
	SOCKET skRet = m_hSocket;
	m_hSocket = INVALID_SOCKET;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));
	m_bHaveAddr = false;
	return skRet;
}

bool TCPSocket::bind(const sockaddr_in& addrBind)
{
	if( SOCKET_ERROR == ::bind(m_hSocket, (sockaddr*)&addrBind, sizeof(sockaddr)))
	{
		int nErr = ::GetLastError();
		//常见的err 为 WSAEADDRINUSE  10048
		Logger::getInstace()->error("TCPSocket >> Bind address faild error id: %d", nErr);
		return false;
	}
	return true;
}

bool TCPSocket::listen(int nBacklog /*= 10*/)
{
	if(SOCKET_ERROR == ::listen(m_hSocket, nBacklog))
	{
		int nErr = ::GetLastError();
		//常见的err 为 WSAEADDRINUSE  10048
		Logger::getInstace()->error("TCPSocket >> listen faild error id: %d", nErr);
		return false;
	}
	return true;
}

bool TCPSocket::startServer(int nPort)
{
	sockaddr_in addr = s_getSockAddrIpV4(NULL, nPort);
	if(!this->bind(addr))  return false;

	return this->listen();
}

TCPSocket*	TCPSocket::accept()
{
	TCPSocket *pNewSocket = new TCPSocket;
	sockaddr_in	conAddr;
	memset(&conAddr, 0, sizeof(conAddr));
	int nAddrLen = sizeof(conAddr);

	SOCKET hConnSocket = ::accept(m_hSocket, (sockaddr*)&conAddr, &nAddrLen);
	if(INVALID_SOCKET == hConnSocket)
	{
		Logger::getInstace()->error("TCPSocket >> accept 返回一个INVALID_SOCKET");
		delete pNewSocket;
		return NULL;
	}
	pNewSocket->attach(hConnSocket);
	pNewSocket->setPeerAddr(conAddr);
	return pNewSocket;
}

bool	TCPSocket::connect(const sockaddr_in& addrCon)
{
	if( SOCKET_ERROR == ::connect(m_hSocket, (sockaddr*)&addrCon, sizeof(addrCon)) )
	{
		int nErr = ::GetLastError();
		Logger::getInstace()->error("TCPSocket >> Connect failed error id: %d", nErr);
		return false;
	}
	setPeerAddr(addrCon);
	return true;
}

int	TCPSocket::recv(char* pBuf, int nLen, int nFlag /* =0*/)
{
	int nSize = ::recv(m_hSocket, pBuf, nLen, nFlag);
	if(nSize == 0)
	{
		return -2;
	}
	else if(nSize == SOCKET_ERROR)
	{
		return -1;
	}
	return nSize;
}

int	TCPSocket::send(const char *pBuf,const int nLen, int nFlag/* =0*/)
{
	int nSize;
	if( SOCKET_ERROR == (nSize = ::send(m_hSocket, pBuf, nLen, nFlag)) )
	{
		int nErr = ::GetLastError();
		return -1;
	}
	return nSize;
}

bool TCPSocket::close()
{
	if(INVALID_SOCKET == m_hSocket)
		return true;

	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
	return true;
}

bool TCPSocket::disConnect()
{
	this->shutdown();
	return this->close();
}

bool TCPSocket::getLocalAddr(std::string& strIpAddr, u_short& usPort)
{
	sockaddr_in addr;
	int nLen = sizeof(addr);
	if(0 == getsockname(m_hSocket, (sockaddr*)&addr, &nLen))
	{
		s_getSockAddrIpV4(addr, strIpAddr, usPort);
		return true;
	}
	return false;
}

bool TCPSocket::getPeerAddr(std::string& strIpAddr, u_short& usPort)
{
	if(!m_bHaveAddr && !initPeerAddress() )
		return false;

	s_getSockAddrIpV4(m_addrPeer, strIpAddr, usPort);
	return true;
}

bool TCPSocket::setPeerAddr(const sockaddr_in& addrPeer)
{
	m_addrPeer = addrPeer;
	m_bHaveAddr = true;
	return true;
}

SOCKET	TCPSocket::getSocket()
{
	return m_hSocket;
}

bool TCPSocket::updateAcceptContext()
{
	return 0 == setsockopt( m_hSocket,
		SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char *)&(m_hSocket), sizeof(m_hSocket) );
}

bool TCPSocket::initPeerAddress()
{
	assert( INVALID_SOCKET != m_hSocket );
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);

	if ( SOCKET_ERROR == getpeername( m_hSocket,
		(sockaddr*)&sockAddr, &nSockAddrLen ) ) return false;

	m_addrPeer = sockAddr;
	m_bHaveAddr = true;
	return true;
}

bool TCPSocket::shutdown(int how /*= SD_BOTH*/)
{
	if(m_hSocket == INVALID_SOCKET)	return true;
	return 0 == ::shutdown(m_hSocket, how);
}

//////////////////////////////////////////////////////////////////////////
//用于自动装载 SocketLib 用
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
	static bool			s_loadSockLib(int nHigh = 2, int nLow = 2);
	static bool			s_destroySockLib();
private:
	static	bool		s_bLoadedSockLib;
};
bool SocketLibLoadHelper::s_bLoadedSockLib = false;
bool SocketLibLoadHelper::s_loadSockLib(int nHigh /* = 2 */, int nLow /* = 2 */)
{
	if( s_bLoadedSockLib ) //已经加载过，直接返回
		return true;
	s_bLoadedSockLib = true;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( nHigh, nLow );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
		return false;

	if ( LOBYTE( wsaData.wVersion ) != nHigh 
		|| HIBYTE( wsaData.wVersion ) != nLow )
	{
		WSACleanup( );
		return false;
	}
	return true;
}
bool SocketLibLoadHelper::s_destroySockLib()
{
	if( !s_bLoadedSockLib ) //还未加载过，或已经卸载了
		return true; 

	s_bLoadedSockLib = false;
	if(0 != WSACleanup())
		return false;

	return true;
}

SocketLibLoadHelper g_socketLibLoadHelper; //定义一个全局变量，使运行环境自动装载和卸载 SocketLib
//////////////////////////////////////////////////////////////////////////