#include "socket_option.h"

#if (CUR_PLATFROM == PLATFORM_WIN32)
	#include <MSWSock.h>
#elif (CUR_PLATFROM == PLATFORM_UNKNOW)
	#include <fcntl.h>
#endif

SocketOption::SocketOption()
{

}

SocketOption::~SocketOption()
{

}

bool SocketOption::setResueAddr(Socket& sock, bool bReuse)
{
	DWORD opt = bReuse ? 1 : 0;

	return setoption(sock, SO_REUSEADDR, &opt, sizeof(opt));
}

bool SocketOption::isReuseAddr(Socket& sock)
{
	DWORD opt = 0;
	socklen_t optlen = sizeof(opt);

	VERIFY(getoption(sock, SO_REUSEADDR, &opt, &optlen));

	return !(opt == 0);
}

bool SocketOption::isListening(Socket& sock)
{
	DWORD opt = 0;
	socklen_t optlen = sizeof(opt);

	VERIFY(getoption(sock, SO_ACCEPTCONN, &opt, &optlen));

	return  !(opt == 0);
}

bool SocketOption::getRemoteAddr(Socket& sock, InterAddress& remoteAddr)
{
	socklen_t nLen = remoteAddr.getAddrLen();
	bool bRet = (0 == getpeername(sock.getHandle(), remoteAddr.getAddress(), &nLen));

#ifdef _DEBUG
	int32_t nErr = ::GetLastError();
#endif
	return bRet;
}

bool SocketOption::getLocalAddr(Socket& sock, InterAddress& localAddr)
{
	socklen_t nLen = localAddr.getAddrLen();
	bool bRet = (0 == getsockname(sock.getHandle(), localAddr.getAddress(), &nLen));

#ifdef _DEBUG
	int32_t nErr = ::GetLastError();
#endif
	return bRet;
}

#if (CUR_PLATFROM == PLATFORM_WIN32)
int32_t SocketOption::getConnectTime(Socket& sock)
{
	int32_t nSecondConnect = -1;
	socklen_t nOptLen = sizeof(nSecondConnect);

	VERIFY(getoption(sock, SO_CONNECT_TIME, &nSecondConnect, &nOptLen));

	return nSecondConnect;
}
#endif//_WIN32

int32_t SocketOption::getRecvBufSize(Socket& sock)
{
	int32_t nOptVal = 0;
	socklen_t nOptLen = sizeof(nOptVal);
	
	VERIFY(getoption(sock, SO_RCVBUF, &nOptVal, &nOptLen));

	return nOptVal;
}

int32_t SocketOption::getSendBufSize(Socket& sock)
{
	int32_t nOptVal = 0;
	socklen_t nOptLen = sizeof(nOptVal);

	VERIFY(getoption(sock, SO_SNDBUF, &nOptVal, &nOptLen));

	return nOptVal;
}

Socket::SockType SocketOption::getSocketType(Socket& sock)
{
	int32_t nOptVal = 0;
	socklen_t nOptLen = sizeof(nOptVal);

	VERIFY(getoption(sock, SO_TYPE, &nOptVal, &nOptLen));

	return (Socket::SockType)nOptVal;
}

/*
http://msdn.microsoft.com/en-us/library/windows/desktop/ms738573(v=vs.85).aspx
*/
bool  SocketOption::setBlockMode(Socket& sock, bool bBlock)
{
	SOCKET_HANDLE hSock = sock.getHandle();
	if (INVALID_SOCKET_HANDLE == hSock)
		return false;

#if (CUR_PLATFROM == PLATFORM_WIN32)
	sock.setBlocked(bBlock);
	u_long iMode = bBlock ? 0 : 1;
	return (SOCKET_ERROR != ioctlsocket(hSock, FIONBIO, &iMode));

#elif (CUR_PLATFROM == PLATFORM_UNKNOW)

	int32_t flag;
	if (flag = fcntl(hSock, F_GETFL, 0) < 0)
		return false;
	
	SET_DEL_BIT( flag, O_NONBLOCK, !bBlock );

	if (fcntl(hSock, F_SETFL, flag) < 0)
		return false;

	return true;
#endif
}

bool  SocketOption::isBlockMode(Socket& sock)
{
	SOCKET_HANDLE hSock = sock.getHandle();
	if (INVALID_SOCKET_HANDLE == hSock)
		return false;

#if (CUR_PLATFROM == PLATFORM_WIN32)
	//return (SOCKET_ERROR != ioctlsocket(hSock, FIONBIO, ));
	return sock.isBlocked();
#elif (CUR_PLATFROM == PLATFORM_UNKNOW)

	int32_t flag;
	if (flag = fcntl(hSock, F_GETFL, 0) < 0)
		return false;

	return QUERY_IS_SET_BIT(flag, O_NONBLOCK);

#endif

}
bool  SocketOption::setRecvBufSize(Socket& sock, int32_t nBufSize)
{
	return setoption(sock, SO_RCVBUF, &nBufSize, sizeof(nBufSize));
}

bool  SocketOption::setSendBufSize(Socket& sock, int32_t nBufSize)
{
	return setoption(sock, SO_SNDBUF, &nBufSize, sizeof(nBufSize));
}

bool SocketOption::setKeepAlive(Socket& sock, bool bKeepAlive)
{
	int keepalive = bKeepAlive ? 1 : 0;

	return setoption(sock, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
}
bool SocketOption::isKeepAlive(Socket& sock)
{
	int32_t nOptVal = 0;
	socklen_t nOptLen = sizeof(nOptVal);

	VERIFY(getoption(sock, SO_TYPE, &nOptVal, &nOptLen));

	return nOptVal == 1;
}
bool SocketOption::setoption(Socket& sock, uint32_t optname, const void *optval, socklen_t optlen)
{
	SOCKET_HANDLE hSocket = sock.getHandle();
	if (INVALID_SOCKET_HANDLE == hSocket)
		return false;

	bool bRet = (SOCKET_ERROR != ::setsockopt(hSocket,
		SOL_SOCKET, optname,
		(char*)optval,
		optlen));

	int32_t nErr = ::GetLastError();

	return bRet;
}

bool SocketOption::getoption(Socket& sock, uint32_t optname, void* optval, socklen_t* optlen)
{
	SOCKET_HANDLE hSocket = sock.getHandle();
	if (INVALID_SOCKET_HANDLE == hSocket)
		return false;

	bool bRet = (SOCKET_ERROR != ::getsockopt(hSocket,
		SOL_SOCKET, optname,
		(char*)optval,
		optlen));

	int32_t nErr = ::GetLastError();

	return bRet;
}

