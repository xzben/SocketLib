/********************************************************************************
*	文件名称:	common.h														*
*	创建时间：	2015/04/14														*
*	作   者 :	xzben															*
*	文件功能:	系统中通用的定义								*
*********************************************************************************/
#ifndef __2015_04_14_COMMON_H__
#define __2015_04_14_COMMON_H__

#include <cstdint>
#include <queue>
#include <string>
#include "config.h"

/*************************************************
**	系统中使用的 DELETE 安全宏定义				**
**************************************************/
#define	SAFE_DELETE(__ptr)					if( nullptr != (__ptr) ) { delete __ptr; __ptr = nullptr; }
#define SAFE_DELETE_ARR(__ptr)				if( nullptr != (__ptr) ) { delete[] __ptr; __ptr = nullptr; }

#define socketlib_error( err_msg )					(err_msg);
typedef uint32_t	SERVER_HANDLE; //服务标示句柄
typedef uint32_t	SESSION_ID; //每个服务的一个操作标示ID

enum : SERVER_HANDLE
{
	SERVER_HANDLE_LOG = 1,
	SERVER_HANDLE_TIMER,
	SERVER_HANDLE_MASTER,
	//////////////////////////////////////////////////////////////////////////
	//must be last
	SYS_SERVER_HANLE_END,
};

enum LOG_LEVEL
{
	LOG_ALL = 0,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_OFF,
};

enum TaskType
{
	EMPTY = 0,
	TIME_OUT,
	SYS_LOG,
};

#endif//__2015_04_14_COMMON_H__