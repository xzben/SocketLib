#ifndef __2014_01_04_EPOLL_H__
#define __2014_01_04_EPOLL_H__

#include "IODriver.h"

#if(CUR_PLATFROM == PLATFROM_LINUX)

class EpollEventDataManager;
class EpollDriver : public IODriver
{
public:
	EpollDriver();
	virtual ~EpollDriver();

	virtual int32_t poll_event_process(IOEvent *events, int32_t max, int waittime = -1) override;
	virtual int32_t poll_add(SOCKET_HANDLE sock) override;
	virtual int32_t poll_del(SOCKET_HANDLE sock) override;
	virtual int32_t poll_listen(SOCKET_HANDLE sock) override;
	virtual int32_t poll_connect(const short port, const char* ip) override;
	virtual int32_t poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz) override;
	virtual int32_t poll_recv(SOCKET_HANDLE sock) override;
protected:
	
private:
	SOCKET_HANDLE  m_epollHandle;
	
	EpollEventDataManager* m_pEvtDataManager;
};

#endif//(CUR_PLATFROM == PLATFROM_LINUX)

#endif//__2014_01_04_EPOLL_H__
