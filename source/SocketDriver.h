/********************************************************************************
*	文件名称:	SocketDriver.h													*
*	创建时间：	2014/03/29														*
*	作   者 :	xzben															*
*	文件功能:	系统网络层的实现													*
*********************************************************************************/
#ifndef __2015_03_29_SOCKET_DRIVER_H__
#define __2015_03_29_SOCKET_DRIVER_H__

#include "SingleInstance.h"
#include "Thread.h"
#include "package.h"
#include <mutex>
#include <unordered_map>
#include <set>

#if (CUR_PLATFROM == PLATFORM_WIN32)
	#include "iocp.h"
#else
	#include "epoll.h"
#endif


class CSocketDriver : public CThread, public SingleInstance<CSocketDriver>
{
#if (CUR_PLATFROM == PLATFORM_WIN32)
	typedef IOCPDriver  ImpDirver;
	friend class IOCPDriver;
#else
	typedef EpollDriver ImpDirver;
	friend class EpollDriver;
#endif
	friend class CWatchDog;
public:
	SOCKET_HANDLE	 listen(SERVER_HANDLE server, const short port, const char* ip = nullptr, int backlog = 10);
	int32_t	 connect(const short port, const char* ip);
	int32_t  send(SOCKET_HANDLE sock, void* buffer, int sz);
	int32_t  accept(SERVER_HANDLE server, SOCKET_HANDLE sock);
	int32_t  reject(SOCKET_HANDLE sock);
	void	 close(SOCKET_HANDLE sock);
protected:
	friend class SingleInstance<CSocketDriver>;
	CSocketDriver();
	virtual ~CSocketDriver();
	virtual void initInstance() override;
	virtual void run()override;

	Package* getSocketPackage(SOCKET_HANDLE sock);
	Package* resetSocketPackage(SOCKET_HANDLE sock);
	void	handleAccept(IOEvent &event);
	void	handleRecv(IOEvent &event);
	void	handleSend(IOEvent &event);
	void	handleClose(IOEvent &event);

	SERVER_HANDLE getListenServer(SOCKET_HANDLE listenSock);
	SOCKET_HANDLE getListenSocket(SERVER_HANDLE handle);
protected:
	bool addClientSocket(SOCKET_HANDLE sock);
	void addSocketServerMap(SERVER_HANDLE handle, SOCKET_HANDLE sock);
	void delSocketServerMap(SOCKET_HANDLE sock);
private:
	ImpDirver	*m_imp;
	typedef std::unordered_map<SOCKET_HANDLE, Package*>  Sock2Package;
	Sock2Package	m_clientPackages;
	std::set<SOCKET_HANDLE> m_clientSet;

	typedef std::unordered_map<SOCKET_HANDLE, SERVER_HANDLE>  ListenSock2Server;
	typedef std::unordered_map<SERVER_HANDLE, SOCKET_HANDLE>  Server2ListenSock;
	ListenSock2Server	m_listenSock2Servers;
	Server2ListenSock   m_listenServers2Sock;
	std::mutex			m_lockListenServers;
	
};
#endif//__2015_03_29_SOCKET_DRIVER_H__
