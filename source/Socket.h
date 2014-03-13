#ifndef __2014_03_02_CSOCKET_H__
#define __2014_03_02_CSOCKET_H__

#include "CObject.h"

/*
*	Socket 类封装了Socket常见的TCP协议的一些操作
*/
class TCPSocket : public NoCopyable
{
public:
	static sockaddr_in	s_getSockAddrIpV4(const char* szIpAddr, const u_short usPort);
	static void			s_getSockAddrIpV4(const sockaddr_in addr, std::string &szIpAddr,  u_short &usPort);
public:
	TCPSocket();
	virtual ~TCPSocket();
	//初始化套接字
	bool	init(const int nAf = AF_INET, const int nProtocl = 0);
	//获取套接字绑定的本地地址
	bool	getLocalAddr(std::string& strIpAddr, u_short& usPort);
	//获取连接成功后的套接字的连接对方的地址
	bool	getPeerAddr(std::string& strIpAddr, u_short& usPort);
	//人工指定套接字的连接对方的地址
	bool	setPeerAddr(const sockaddr_in& addrPeer);
	bool	attach(SOCKET hSocket);
	SOCKET	dettach();
	bool	bind(const sockaddr_in& addrBind);
	bool	listen(int nBacklog = 10);
	//将套接字绑定到指定端口并启动监听服务
	bool	startServer(int nPort);
	TCPSocket*	accept();
	bool	connect(const sockaddr_in& addrCon);
	int		recv(char* pBuf, int nLen, int nFlag =0);
	int		send(const char *pBuf,const int nLen, int nFlag = 0);
	SOCKET	getSocket();
	bool	initPeerAddress();
	//使用IOCP 时需要在accept后通过此函数更新连接套接字的属性
	bool	updateAcceptContext();
	bool	shutdown(int how = SD_BOTH);
	bool	close();
	bool	disConnect();
protected:
	SOCKET			m_hSocket;	//套接字句柄
	sockaddr_in		m_addrPeer; //套接字连接后，与之连接的地址
	bool			m_bHaveAddr;//是否包含连接对象的地址
};

#endif//__2014_03_02_CSOCKET_H__