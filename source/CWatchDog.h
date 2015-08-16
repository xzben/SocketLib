/********************************************************************************
*	文件名称:	CWatchDog.h													*
*	创建时间：	2015/07/26														*
*	作   者 :	xzben															*
*	文件功能:	看门狗服务													*
*********************************************************************************/
#ifndef __2015_07_26_WATCHDOG_H__
#define __2015_07_26_WATCHDOG_H__

#include "Server.h"
#include "SingleInstance.h"
#include "socket.h"

class Package;

class CWatchDog : public CServer
{
public:
	CWatchDog(short prot, const char* ip = nullptr);
	virtual ~CWatchDog();

	void	connect(short port, const char* ip);

protected:
	virtual void excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)override;

	virtual void handleAccept(SOCKET_HANDLE sock, std::string &RemoteIp);
	virtual void handleMsg(SOCKET_HANDLE sock, Package* package);
	virtual void handleClose(SOCKET_HANDLE sock);
	virtual void handleConnect(SOCKET_HANDLE sock);

private:
	SOCKET_HANDLE m_listenSock;
};
#endif//__2015_07_26_WATCHDOG_H__