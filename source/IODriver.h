#ifndef __2014_01_04_IODRIVER_H__
#define __2014_01_04_IODRIVER_H__

#include "socket.h"
#include "package.h"

typedef enum enum_SocketIOType
{
	IO_UNKNOW	= 0,	  //未知
	IO_Write	= 1,      // 写
	IO_Read		= 2,	  // 读
	IO_ReadPart = 4,	  // 读取了部分内容
	IO_Accept	= 8,	  // 接收连接
	IO_Connect	= 16,	  // 链接操作
	IO_Close	= 32,	  // 套接字关闭
}SocketIOType;



#if (CUR_PLATFROM == PLATFORM_WIN32)
	#define  MAX_EVENT	1
#elif (CUR_PLATFROM == PLATFROM_LINUX)
	#define  MAX_EVENT	64
#endif


typedef struct st_IOEvent
{
	st_IOEvent()
	{
		evt_sock = INVALID_SOCKET;
		acpt_sock = INVALID_SOCKET;
		evt_type = IO_UNKNOW;
		package = nullptr;
	}

	SOCKET_HANDLE evt_sock;
	SocketIOType  evt_type;
	SOCKET_HANDLE acpt_sock;
	Package*	  package;
}IOEvent;

class IODriver
{
public:
	IODriver(){};
	virtual ~IODriver(){};

	virtual int32_t poll_event_process(IOEvent *events, int32_t max, int waittime = -1) = 0;
	virtual int32_t poll_add(SOCKET_HANDLE sock) = 0;
	virtual int32_t poll_del(SOCKET_HANDLE sock) = 0;
	virtual int32_t poll_listen(SOCKET_HANDLE sock) = 0;
	virtual int32_t poll_connect(const short port, const char* ip) = 0;
	virtual int32_t poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz) = 0;
	virtual int32_t poll_recv(SOCKET_HANDLE sock) = 0;

};

#endif//__2014_01_04_IODRIVER_H__