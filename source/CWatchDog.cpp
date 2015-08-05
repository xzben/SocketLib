#include "CWatchDog.h"
#include "SocketDriver.h"
#include "GlobalController.h"

CWatchDog::CWatchDog(short port, const char* ip /*=nullptr*/)
{
	m_listenSock = CSocketDriver::getInstance()->listen(getServerHandle(), port, ip);
	if (m_listenSock == INVALID_SOCKET)
	{
		LOG_WARN("CWatchDog Listen failed in port[%d] ip[%s]", port, ip == nullptr ? "nullptr" : ip);
	}
	else
	{
		LOG_INFO("CWatchDog Listen in port[%d] ip[%s]", port, ip == nullptr ? "" : ip);
	}
}

CWatchDog::~CWatchDog()
{
	//服务结束时关闭监听端口
	Socket sock(m_listenSock);
	sock.close();
	CSocketDriver::getInstance()->delSocketServerMap(m_listenSock);
}

void CWatchDog::excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)
{
	ASSERT(type == TSK_SOCKET);
	SocketIOType ioType = pData->pop<SocketIOType, 0>(IO_UNKNOW);
	SOCKET_HANDLE sock = pData->pop<SOCKET_HANDLE, 1>(INVALID_SOCKET);

	switch (ioType)
	{
		case IO_Accept:
		{
			InterAddress remoteAddress = pData->pop<InterAddress, 2>(InterAddress());
			char ip[20];
			int32_t port;
			remoteAddress.getAddress(ip, port);
			std::string strIp(ip);
			handleAccept(sock, strIp);
			break;
		}
		case IO_Read:
		{
			Package* package = pData->pop<Package*, 2>(nullptr);
			handleMsg(sock, package);
			Package::free(package);
			break;
		}
		case IO_Close:
		{
			handleClose(sock);
			break;
		}
	}
}

void CWatchDog::handleAccept(SOCKET_HANDLE sock, std::string &RemoteIp)
{
	static int count = 0;
	LOG_INFO("handle socket accept [%d] address[%s] [%d]", sock, RemoteIp.c_str(), count++);
	CSocketDriver::getInstance()->accept(getServerHandle(), sock);

}

void CWatchDog::handleClose(SOCKET_HANDLE sock)
{
	static int count = 0;
	LOG_INFO("handle socket close [%d] [%d]", sock, count++);
}

void CWatchDog::handleMsg(SOCKET_HANDLE sock, Package* package)
{
	PackageSize head = package->getHead();
	void* buff = package->getBuffer();
	std::string str((char*)buff, head);
	LOG_INFO("handle socket msg [%d]", sock);
	LOG_INFO("content: head[%d | %s]", head, str.c_str());
}