#ifndef __2014_01_04_IOCP_H__
#define __2014_01_04_IOCP_H__

#include "IODriver.h"

#if (CUR_PLATFROM == PLATFORM_WIN32)

class IOCPDriver : public IODriver
{
public:
	IOCPDriver();
	virtual ~IOCPDriver();

	virtual int32_t poll_event_process(IOEvent *events, int32_t max, int waittime = -1) override;
	virtual int32_t poll_add(SOCKET_HANDLE sock) override;
	virtual int32_t poll_del(SOCKET_HANDLE sock) override;
	virtual int32_t poll_listen(SOCKET_HANDLE sock) override;
	virtual int32_t poll_connect(SERVER_HANDLE handle, const short port, const char* ip) override;
	virtual int32_t poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz) override;
	virtual int32_t poll_recv(SOCKET_HANDLE sock) override;
protected:
	int32_t		recv(SOCKET_HANDLE sock, Package* package = nullptr);
	int32_t		send(SOCKET_HANDLE sock, Package* package);
	int32_t		accept(SOCKET_HANDLE sock);
protected:
	HANDLE	m_hCompletionPort;
};

#endif // (CUR_PLATFROM == PLATFORM_WIN32)

#endif//__2014_01_04_IOCP_H__