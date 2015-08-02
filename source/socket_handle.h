/*************************************************************************
*	文件名称:	socket_handle.h											 *
*	创建时间：	2014/03/17												 *
*	作   者 :	xzben													 *
*	文件功能:	本文件主要用于socket相关的头文件包含和参数的跨平台兼容设计	 *
*************************************************************************/

#include "common.h"

#ifndef __2014_03_17_SOCKET_HANDLE_H__
#define __2014_03_17_SOCKET_HANDLE_H__

//SOCKET 句柄类型
#if (CUR_PLATFROM == PLATFORM_WIN32)
	#include<winsock2.h>
	#include <MSWSock.h>
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment ( lib, "mswsock.lib")
////////////////////////////////////////////////////////////
//	统一 socket shutdown 的参数
	#define	SHUT_RD					SD_RECEIVE
	#define	SHUT_WR					SD_SEND
	#define	SHUT_RDWR				SD_BOTH
////////////////////////////////////////////////////////
	typedef SOCKET					SOCKET_HANDLE;
	#define	INVALID_SOCKET_HANDLE			INVALID_SOCKET
#elif (CUR_PLATFROM ==PLATFROM_LINUX)
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/select.h>
	#include <sys/epoll.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <errno.h>
////////////////////////////////////////////////////////////
//	统一 socket shutdown 的参数
	#define	SD_RECEIVE				SHUT_RD					
	#define	SD_SEND					SHUT_WR					
	#define	SD_BOTH					SHUT_RDWR	
////////////////////////////////////////////////////////////
	typedef int32_t					SOCKET_HANDLE;
	#define	INVALID_SOCKET_HANDLE			-1
	#define	SOCKET_ERROR				-1
	extern int32_t 	GetLastError();
#endif//平台相关

#endif//__2014_03_17_SOCKET_HANDLE_H__
