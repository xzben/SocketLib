/********************************************************************************
*	文件名称:	Socket.h														*
*	创建时间：	2014/03/18														*
*	作   者 :	xzben															*
*	文件功能:	本文件主要实现一个和平台无关的Socket类								*		
*	error list: http://msdn.microsoft.com/en-us/library/ms681391(v=vs.85).aspx	*
*********************************************************************************/

#include "socket_addr.h"
#include "time_value.h"

#ifndef __2014_03_18_SOCKET_H__
#define __2014_03_18_SOCKET_H__

class TimeValue;
class InterAddress;
class Socket;
/*
*		平台无关的Socket类
*/
class Socket
{
public:
	enum SockType : DWORD
	{
		SOCK_TCP	=	 SOCK_STREAM,
		SOCK_UDP	=	 SOCK_DGRAM,
	};
	
	Socket();
	Socket(SOCKET_HANDLE hSocket);
	Socket(const SockType type, const int32_t nAf = AF_INET, const int32_t nProtocl = 0);
	virtual ~Socket();

 	void			attach(SOCKET_HANDLE hSocket);
 	SOCKET_HANDLE	dettach();
 	SOCKET_HANDLE	getHandle();
	bool			isValidTcp();

	bool			open(const SockType type, const int32_t nAf = AF_INET, const int32_t nProtocl = 0);
 	bool			shutdown(int32_t how = SD_BOTH);
 	bool			close();
	bool			bind(const InterAddress& addrBind);
 	bool			listen(int32_t nBacklog = 10);
 	
	bool			accept(Socket& sockCon, InterAddress* remoteAddr = nullptr);
	bool			accept(const TimeValue& tmVal, Socket& sockCon, InterAddress* remoteAddr = nullptr);

	bool			connect(const InterAddress& addrCon);
	bool			connect(const InterAddress& addrCon, const TimeValue& tmVal);

	int32_t			getReadyStatus(const TimeValue& tmVal, bool *pReadReady = nullptr,
								  bool* pWriteReady = nullptr, bool* pExceptReady = nullptr);
	bool			isReadReady(const TimeValue& tmVal);
	bool			isWriteReady(const TimeValue& tmVal);

	int32_t			recv(void* pBuf, int32_t nLen, int32_t nFlag = 0);
	int32_t			recv(void* pBuf, int32_t nLen, const TimeValue& tmVal, int32_t nFlag = 0);

	int32_t			send(const void *pBuf, const int32_t nLen, int32_t nFlag = 0);
	int32_t			send(const void *pBuf, const int32_t nLen, const TimeValue& tmVal, int32_t nFlag = 0);

	int32_t			recvfrom(void* pBuf, int32_t nLen, InterAddress& addFrom, int32_t nFlag = 0);
	int32_t			recvfrom(void* pBuf, int32_t nLen, InterAddress& addFrom, const TimeValue& tmVal, int32_t nFlag = 0);

	int32_t			sendto(const void *pBuf, const int32_t nLen, const InterAddress& addrTo, int32_t nFlag = 0);
	int32_t			sendto(const void *pBuf, const int32_t nLen, const InterAddress& addrTo, const TimeValue& tmVal, int32_t nFlag = 0);
	
	bool			setBlocked(bool bIsBlock);
	bool			isBlocked();

protected:
	SOCKET_HANDLE	m_hSocket = INVALID_SOCKET_HANDLE;	//套接字句柄
#if (CUR_PLATFROM == PLATFORM_WIN32)
public:
	bool			updateAcceptContext();
	bool			updateConnectContext();
protected:
	// 由于window下未找到合适的判断套接字是否阻塞的方式，目前先用这个标记来自行实现一下
	// window 下默认为阻塞套接字
	bool			m_bIsBlock = true;
#else

#endif // _WIN32

};
#endif//__2014_03_18_SOCKET_H__
