/********************************************************************************
*	文件名称:	sockoption.h													*
*	创建时间：	2014/03/23														*
*	作   者 :	xzben															*
*	文件功能:	封装socket配置option相关的操作									*
*********************************************************************************/

/************************************************************************
* socket option 的配制参数及其说明
Option					Get		Set		Optval	type	Description
PVD_CONFIG				yes		yes		char []	An opaque data structure object containing configuration information for the service provider. This option is implementation dependent.
SO_ACCEPTCONN			yes				DWORD (boolean)	Returns whether a socket is in listening mode. This option is only Valid for connection-oriented protocols.
SO_BROADCAST			yes		yes		DWORD (boolean)	Configure a socket for sending broadcast data. This option is only Valid for protocols that support broadcasting (IPX and UDP, for example).
SO_BSP_STATE			yes				CSADDR_INFO	Returns the local address, local port, remote address, remote port, socket type, and protocol used by a socket. See the SO_BSP_STATE reference for more information.
SO_CONDITIONAL_ACCEPT	yes		yes		DWORD (boolean)	Indicates if incoming connections are to be accepted or rejected by the application, not by the protocol stack. See the SO_CONDITIONAL_ACCEPT reference for more information.
SO_CONNDATA				yes		yes		char []	Additional data, not in the normal network data stream, that is sent with network requests to establish a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_CONNDATALEN			yes				ULONG	The length, in bytes, of additional data, not in the normal network data stream, that is sent with network requests to establish a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_CONNECT_TIME			yes				ULONG	Returns the number of seconds a socket has been connected. This option is only valid for connection-oriented protocols.
SO_CONNOPT				yes		yes		char []	Additional connect option data, not in the normal network data stream, that is sent with network requests to establish a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_CONNOPTLEN			yes				ULONG	The length, in bytes, of connect option data, not in the normal network data stream, that is sent with network requests to establish a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_DISCDATA				yes		yes		char []	Additional data, not in the normal network data stream, that is sent with network requests to disconnect a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_DISCDATALEN			yes				ULONG	The length, in bytes, of additional data, not in the normal network data stream, that is sent with network requests to disconnect a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_DISCOPT				yes		yes		char []	Additional disconnect option data, not in the normal network data stream, that is sent with network requests to disconnect a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_DISCOPTLEN			yes				ULONG	The length, in bytes, of additional disconnect option data, not in the normal network data stream, that is sent with network requests to disconnect a connection. This option is used by legacy protocols such as DECNet, OSI TP4, and others. This option is not supported by the TCP/IP protocol in Windows.
SO_DEBUG				yes		yes		DWORD (boolean)	Enable debug output. Microsoft providers currently do not output any debug information.
SO_DONTLINGER			yes		yes		DWORD (boolean)	Indicates the state of the l_onoff member of the linger structure associated with a socket. If this member is nonzero, a socket remains open for a specified amount of time after a closesocket function call to enable queued data to be sent. This option is only valid for reliable, connection-oriented protocols.
SO_DONTROUTE			yes		yes		DWORD (boolean)	Indicates that outgoing data should be sent on whatever interface the socket is bound to and not a routed on some other interface. This option is only Valid for message-oriented protocols. Microsoft providers silently ignore this option and always consult the routing table to find the appropriate outgoing interface.
SO_ERROR				yes				DWORD	Returns the last error code on this socket. This per-socket error code is not always immediately set.
SO_EXCLUSIVEADDRUSE		yes		yes		DWORD (boolean)	Prevents any other socket from binding to the same address and port. This option must be set before calling the bind function. See the SO_EXCLUSIVEADDRUSE reference for more information.
SO_GROUP_ID				yes				unsigned int	This socket option is reserved and should not be used.
SO_GROUP_PRIORITY		yes		yes		int	This socket option is reserved and should not be used.
SO_KEEPALIVE			yes		yes		DWORD (boolean)	Enables keep-alive for a socket connection. Valid only for protocols that support the notion of keep-alive (connection-oriented protocols). For TCP, the default keep-alive timeout is 2 hours and the keep-alive interval is 1 second. The default number of keep-alive probes varies based on the version of Windows. See the SO_KEEPALIVE reference for more information.
SO_LINGER				yes		yes		struct linger	Indicates the state of the linger structure associated with a socket. If the l_onoff member of the linger structure is nonzero, a socket remains open for a specified amount of time after a closesocket function call to enable queued data to be sent. The amount of time, in seconds, to remain open is specified in the l_linger member of the linger structure. This option is only valid for reliable, connection-oriented protocols.
SO_MAX_MSG_SIZE			yes				ULONG	Returns the maximum outbound message size for message-oriented sockets supported by the protocol. Has no meaning for stream-oriented sockets.
SO_MAXDG				yes				ULONG	Returns the maximum size, in bytes, for outbound datagrams supported by the protocol. This socket option has no meaning for stream-oriented sockets.
SO_MAXPATHDG			yes				ULONG	Returns the maximum size, in bytes, for outbound datagrams supported by the protocol to a given destination address. This socket option has no meaning for stream-oriented sockets. Microsoft providers may silently treat this as SO_MAXDG.
SO_OOBINLINE			yes		yes		DWORD (boolean)	Indicates that out-of-bound data should be returned in-line with regular data. This option is only valid for connection-oriented protocols that support out-of-band data.
SO_OPENTYPE				yes		yes		DWORD	Once set, affects whether subsequent sockets that are created will be non-overlapped. The possible values for this option are SO_SYNCHRONOUS_ALERT and SO_SYNCHRONOUS_NONALERT. This option should not be used. Instead use the WSASocket function and leave the WSA_FLAG_OVERLAPPED bit in the dwFlags parameter turned off.
SO_PORT_SCALABILITY		yes		yes		DWORD (boolean)	Enables local port scalability for a socket by allowing port allocation to be maximized by allocating wildcard ports multiple times for different local address port pairs on a local machine. See the SO_PORT_SCALABILITY reference for more information.
SO_PROTOCOL_INFO		yes				WSAPROTOCOL_INFO	This option is defined to the SO_PROTOCOL_INFOW socket option if the UNICODE macro is defined. If the UNICODE macro is not defined, then this option is defined to the SO_PROTOCOL_INFOA socket option.
SO_PROTOCOL_INFOA		yes				WSAPROTOCOL_INFOA	Returns the WSAPROTOCOL_INFOA structure for the given socket
SO_PROTOCOL_INFOW		yes				WSAPROTOCOL_INFOW	Returns the WSAPROTOCOL_INFOW structure for the given socket
SO_RCVBUF				yes		yes		DWORD	The total per-socket buffer space reserved for receives. This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of the TCP receive window.
SO_RCVLOWAT				yes		yes		DWORD	A socket option from BSD UNIX included for backward compatibility. This option sets the minimum number of bytes to process for socket input operations.
This option is not supported by the		Windows TCP/IP provider. If this option is used on Windows Vista and later, the getsockopt and setsockopt functions fail with WSAEINVAL. On earlier versions of Windows, these functions fail with WSAENOPROTOOPT.
SO_RCVTIMEO				yes		yes		DWORD
The timeout, in milliseconds, for blocking receive calls. The default for this option is zero, which indicates that a receive operation will not time out. If a blocking receive call times out, the connection is in an indeterminate state and should be closed.
If the socket is created using the WSASocket function, then the dwFlags parameter must have the WSA_FLAG_OVERLAPPED attribute set for the timeout to function properly. Otherwise the timeout never takes effect.
SO_REUSEADDR			yes		yes	DWORD (boolean)	Allows socket to bind to an address and port already in use. The SO_EXCLUSIVEADDRUSE option can prevent this. Also, if two sockets are bound to the same port the behavior is undefined as to which port will receive packets.
SO_SNDBUF				yes		yes	DWORD	The total per-socket buffer space reserved for sends. This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of a TCP send window.
SO_SNDLOWAT				yes		yes	DWORD	A socket option from BSD UNIX included for backward compatibility. This option sets the minimum number of bytes to process for socket output operations.
This option is not supported by the Windows TCP/IP provider. If this option is used on Windows Vista and later, the getsockopt and setsockopt functions fail with WSAEINVAL. On earlier versions of Windows, these functions fail with WSAENOPROTOOPT.
SO_SNDTIMEO				yes		yes	DWORD
The timeout, in milliseconds, for blocking send calls. The default for this option is zero, which indicates that a send operation will not time out. If a blocking send call times out, the connection is in an indeterminate state and should be closed.
If the socket is created using the WSASocket function, then the dwFlags parameter must have the WSA_FLAG_OVERLAPPED attribute set for the timeout to function properly. Otherwise the timeout never takes effect.
SO_TYPE					yes		DWORD	Returns the socket type for the given socket (SOCK_STREAM or SOCK_DGRAM, for example).
SO_UPDATE_ACCEPT_CONTEXT		yes	DWORD (boolean)	This option is used with the AcceptEx function. This option updates the properties of the socket which are inherited from the listening socket. This option should be set if the getpeername, getsockname, getsockopt, or setsockopt functions are to be used on the accepted socket.
SO_UPDATE_CONNECT_CONTEXT		yes	DWORD (boolean)	This option is used with the ConnectEx, WSAConnectByList, and WSAConnectByName functions. This option updates the properties of the socket after the connection is established. This option should be set if the getpeername, getsockname, getsockopt, setsockopt, or shutdown functions are to be used on the connected socket.
SO_USELOOPBACK			yes		yes	DWORD (boolean)	Use the local loopback address when sending data from this socket. This option should only be used when all data sent will also be received locally.
This option is not supported by the Windows TCP/IP provider. If this option is used on Windows Vista and later, the getsockopt and setsockopt functions fail with WSAEINVAL. On earlier versions of Windows, these functions fail with WSAENOPROTOOPT.                                                                    */
/************************************************************************/
/*
getsockopt:
http://msdn.microsoft.com/en-us/library/windows/desktop/ms738544(v=vs.85).aspx
setsockopt:
http://msdn.microsoft.com/en-us/library/windows/desktop/ms740476(v=vs.85).aspx
*/
#include "socket.h"

#ifndef __2014_03_23_SOCK_OPTION_H__
#define __2014_03_23_SOCK_OPTION_H__


class SocketOption
{
public:
	SocketOption();
	~SocketOption();
	/*
	*	设置/获取 套接字是否能够绑定到一个已经被使用的端口地址上
	*/
	bool setResueAddr(Socket& sock, bool bReuse);
	bool isReuseAddr(Socket& sock);

	/*
	*	获取一个面向连接协议的socket是否处在 listen状态
	*/
	bool isListening(Socket& sock);

	/*
	*	获取 套接字 的 本地地址
	*/
	bool getLocalAddr(Socket& sock, InterAddress& localAddr);

	/*
	*	获取 建立连接的 TCP 套接字 的 远程地址
	*/
	bool getRemoteAddr(Socket& sock, InterAddress& remoteAddr);

#ifdef _WIN32
	/*
	*	获取面向连接协议的套接字当前连接建立的秒数,目前只支持win32
	*/
	int32_t getConnectTime(Socket& sock);
#endif//_WIN32
	/*
	*	获取/设置 发送/结束 缓冲区的size
	*/
	int32_t getRecvBufSize(Socket& sock);
	int32_t getSendBufSize(Socket& sock);
	bool  setRecvBufSize(Socket& sock, int32_t nBufSize);
	bool  setSendBufSize(Socket& sock, int32_t nBufSize);

	/*
	*	获取套接字类型
	*/
	Socket::SockType	getSocketType(Socket& sock);

	/*
	*	设置套接字非阻塞
	*/
	bool	setBlockMode(Socket& sock, bool bBlock);
	bool	isBlockMode(Socket& sock);

	/*
	*/
	bool	setKeepAlive(Socket& sock, bool bKeepAlive);
	bool	isKeepAlive(Socket& sock);
private:
	bool setoption(Socket& sock, uint32_t optname, const void *optval, socklen_t optlen);
	bool getoption(Socket& sock, uint32_t optname, void* optval, socklen_t* optlen);
};

#endif//__2014_03_23_SOCK_OPTION_H__
