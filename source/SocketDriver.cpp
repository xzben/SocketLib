#include "SocketDriver.h"
#include "GlobalController.h"
#include "MasterServer.h"
#include "socket_addr.h"
#include "time_value.h"
#include "WorkerPool.h"
#include "socket_option.h"
#include <iostream>

CSocketDriver::CSocketDriver()
{
	m_imp = new ImpDirver;
}

CSocketDriver::~CSocketDriver()
{
	SAFE_DELETE(m_imp);
}

void CSocketDriver::initInstance()
{

}

Package*	CSocketDriver::resetSocketPackage(SOCKET_HANDLE sock)
{
	Package* ret = nullptr;
	{
		auto it = m_clientSet.find(sock);
		if (it == m_clientSet.end())
			return ret;
	}

	{
		auto it = m_clientPackages.find(sock);
		if (it == m_clientPackages.end())
		{
			ret = Package::create();
			m_clientPackages.insert(std::make_pair(sock, ret));
		}
		else
		{
			ret = it->second;
			if (ret->isFillComplete())
			{
				ret = Package::create();
				it->second = ret;
			}
		}
	}

	return ret;
}

bool CSocketDriver::addClientSocket(SOCKET_HANDLE sock)
{
	auto it = m_clientSet.find(sock);
	if (it == m_clientSet.end())
	{
		m_clientSet.insert(sock);
		return true;
	}

	return false;
}

Package* CSocketDriver::getSocketPackage(SOCKET_HANDLE sock)
{
	Package* ret = nullptr;
	{
		auto it = m_clientSet.find(sock);
		if (it == m_clientSet.end())
			return ret;
	}

	{
		auto it = m_clientPackages.find(sock);
		if (it == m_clientPackages.end())
		{
			ret = Package::create();
			m_clientPackages.insert(std::make_pair(sock, ret));
		}
		else
		{
			ret = it->second;
		}
	}
	
	return ret;
}

SERVER_HANDLE CSocketDriver::getListenServer(SOCKET_HANDLE listenSock)
{
	SERVER_HANDLE ret = INVALID_SERVER_HANDLE;

	m_lockListenServers.lock();
	auto it = m_listenSock2Servers.find(listenSock);
	if (it != m_listenSock2Servers.end())
	{
		ret = it->second;
	}
	m_lockListenServers.unlock();

	return ret;
}

SOCKET_HANDLE CSocketDriver::getListenSocket(SERVER_HANDLE handle)
{
	SOCKET_HANDLE ret = INVALID_SOCKET;

	m_lockListenServers.lock();
	auto it = m_listenServers2Sock.find(handle);
	if (it != m_listenSock2Servers.end())
	{
		ret = it->second;
	}
	m_lockListenServers.unlock();

	return ret;
}

void CSocketDriver::delSocketServerMap(SOCKET_HANDLE sock)
{
	m_lockListenServers.lock();
	auto it = m_listenSock2Servers.find(sock);
	if (it != m_listenSock2Servers.end())
	{
		SERVER_HANDLE handle = it->second;
		auto it1 = m_listenServers2Sock.find(handle);
		if (it1 != m_listenServers2Sock.end())
		{
			m_listenServers2Sock.erase(it1);
		}
		m_listenSock2Servers.erase(it);
	}
	
	m_lockListenServers.unlock();
}

void CSocketDriver::addSocketServerMap(SERVER_HANDLE handle, SOCKET_HANDLE sock)
{
	m_lockListenServers.lock();
	m_listenSock2Servers.insert(std::make_pair(sock, handle));
	m_listenServers2Sock.insert(std::make_pair(handle, sock));
	m_lockListenServers.unlock();
}

SOCKET_HANDLE	CSocketDriver::listen(SERVER_HANDLE server, const short port, const char* ip, int backlog)
{
	Socket sock;
	sock.open(Socket::SOCK_TCP);
	sock.bind(InterAddress(port, ip));
	sock.listen(backlog);
	sock.setBlocked(false);
	SOCKET_HANDLE listenSock = sock.getHandle();
	
	if (m_imp->poll_listen(listenSock) != 0)
	{
		sock.close();
		return INVALID_SOCKET;
	}
	sock.dettach();

	addSocketServerMap(server, listenSock);
	return listenSock;
}

int32_t CSocketDriver::connect(const short port, const char* ip)
{
	return m_imp->poll_connect(port, ip);
}

int32_t  CSocketDriver::send(SOCKET_HANDLE sock, void* buffer, int sz)
{
	return m_imp->poll_send(sock, buffer, sz);
}

int32_t  CSocketDriver::accept(SERVER_HANDLE server, SOCKET_HANDLE sock)
{
	if (0 != m_imp->poll_add(sock) || !addClientSocket(sock) || 0 != m_imp->poll_recv(sock))
	{
		LOG_WARN("accept failed [%d]", sock);
		return -1;
	}
	addSocketServerMap(server, sock);

	return 0;
}

int32_t  CSocketDriver::reject(SOCKET_HANDLE sock)
{
	Socket socket;
	socket.attach(sock);
	socket.close();

	return 0;
}

void CSocketDriver::run()
{
	ThreadGuard guard;
	IOEvent events[MAX_EVENT];

	while (!CGlobalController::getInstance()->isSystemStop())
	{
		int result = m_imp->poll_event_process(events, MAX_EVENT, -1);
		for (int i = 0; i < result; i++)
		{
			switch (events[i].evt_type )
			{
			case IO_Accept:
				handleAccept(events[i]);
				break;
			case IO_Read:
				handleRecv(events[i]);
				break;
			case IO_Write:
				handleSend(events[i]);
				break;
			case IO_Close:
				handleClose(events[i]);
				break;
			case IO_Connect:
				break;
			default:
				break;
			}
		}
	}
}

void CSocketDriver::handleAccept(IOEvent &event)
{
	SOCKET_HANDLE sockAcp = event.acpt_sock;
	SOCKET_HANDLE sockListen = event.evt_sock;
	
	SERVER_HANDLE listenServer = getListenServer(sockListen);
	if (listenServer == INVALID_SERVER_HANDLE)
		return;

	Socket sock;
	sock.attach(sockAcp);

	InterAddress remoteAddress;
	SocketOption option;
	option.getRemoteAddr(sock, remoteAddress);
	sock.dettach();

	CData *pData = new CData;
	pData->push<SocketIOType>(IO_Accept);
	pData->push<SOCKET_HANDLE>(sockAcp);
	pData->push<InterAddress>(remoteAddress);

	CWorkerPool::getInstance()->addTask(listenServer, CTask::create(TSK_SOCKET, 0, SOCKET_DRIVER_HANDLE, pData));
}

void CSocketDriver::handleRecv(IOEvent &event)
{
	SOCKET_HANDLE sock = event.evt_sock;

	SERVER_HANDLE listenServer = getListenServer(sock);
	if (listenServer == INVALID_SERVER_HANDLE)
		return;

	CData *pData = new CData;
	pData->push<SocketIOType>(IO_Read);
	pData->push<SOCKET_HANDLE>(sock);
	pData->push<Package*>(event.package);

	CWorkerPool::getInstance()->addTask(listenServer, CTask::create(TSK_SOCKET, 0, SOCKET_DRIVER_HANDLE, pData));
}

void CSocketDriver::handleSend(IOEvent &event)
{

}

void CSocketDriver::handleClose(IOEvent &event)
{
	SOCKET_HANDLE sock = event.evt_sock;

	SERVER_HANDLE listenServer = getListenServer(sock);
	if (listenServer == INVALID_SERVER_HANDLE)
		return;

	CData *pData = new CData;
	pData->push<SocketIOType>(IO_Close);
	pData->push<SOCKET_HANDLE>(sock);

	CWorkerPool::getInstance()->addTask(listenServer, CTask::create(TSK_SOCKET, 0, SOCKET_DRIVER_HANDLE, pData));
	delSocketServerMap(sock);
}