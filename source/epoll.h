#ifndef __2014_01_04_EPOLL_H__
#define __2014_01_04_EPOLL_H__

#include "IODriver.h"

#if(CUR_PLATFROM == PLATFORM_UNKNOW)

class EpollEventDataManager;
class Package;

class EpollDriver : public IODriver
{
public:
	EpollDriver();
	virtual ~EpollDriver();

	virtual int32_t poll_event_process(IOEvent *events, int32_t max, int waittime = -1) override;
	virtual int32_t poll_add(SOCKET_HANDLE sock) override;
	virtual int32_t poll_del(SOCKET_HANDLE sock) override;
	virtual int32_t poll_listen(SOCKET_HANDLE sock) override;
	virtual int32_t poll_connect(SERVER_HANDLE handle, const short port, const char* ip) override;
	virtual int32_t poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz) override;
	virtual int32_t poll_recv(SOCKET_HANDLE sock) override;
protected:
	int32_t clearEvent(SOCKET_HANDLE sock, uint32_t events, SocketIOType ioType);
	int32_t addEvent(SOCKET_HANDLE sock, SocketIOType ioType, uint32_t events, Package* RecvPackage = nullptr);
	int32_t	recv(SOCKET_HANDLE sock, Package* package = nullptr);
private:
	SOCKET_HANDLE  m_epollHandle;
	
	EpollEventDataManager* m_pEvtDataManager;
};

#endif//(CUR_PLATFROM == PLATFORM_UNKNOW)

#endif//__2014_01_04_EPOLL_H__
