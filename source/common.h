/********************************************************************************
*	文件名称:	common.h														*
*	创建时间：	2015/04/14														*
*	作   者 :	xzben															*
*	文件功能:	系统中通用的定义								*
*********************************************************************************/
#ifndef __2015_04_14_COMMON_H__
#define __2015_04_14_COMMON_H__

#include <cstdint>
#include <cassert>
#include <queue>
#include <string>
#include "config.h"


/*************************************************
**	系统中使用的 ASSERT 宏定义					**
**************************************************/
//只在DEBUG下有效，__express 只有在debug下才执行
#define	ASSERT(__express)					assert((__express));
//__express 总是会执行，但只有debug的时候才会产生 assert
#define VERIFY(__express)					if( !(__express) ) { ASSERT(false); }


/*************************************************
**	系统中使用的 设置状态标记 					**
**************************************************/
#define	SET_BIT(_dwCntrl, _bit)				((_dwCntrl)|(_bit))
#define GET_BIT(_dwCntrl, _bit)				((_dwCntrl)&(_bit))
#define DEL_BIT(_dwCntrl, _bit)				((_dwCntrl)&(~(_bit)))

#define SET_DEL_BIT(_dwCntrl, _bit, _bSet)	((_bSet) ? SET_BIT(_dwCntrl, _bit) : DEL_BIT(_dwCntrl, _bit))
#define QUERY_IS_SET_BIT(_dwCntrl, _bit)	GET_BIT(_dwCntrl, _bit)

#define SET_PTR_VALUE_SAFE(_ptr, _val)		if(nullptr != _ptr) { *_ptr = _val; }
#define SET_PTR_VALUE_SAFE(_ptr, _val)		if(nullptr != _ptr) { *_ptr = _val; }

/*************************************************
**	系统中使用的 DELETE 安全宏定义				**
**************************************************/
#define	SAFE_DELETE(__ptr)					if( nullptr != (__ptr) ) { delete __ptr; __ptr = nullptr; }
#define SAFE_DELETE_ARR(__ptr)				if( nullptr != (__ptr) ) { delete[] __ptr; __ptr = nullptr; }

typedef uint32_t	SERVER_HANDLE; //服务标示句柄
typedef uint32_t	SESSION_ID; //每个服务的一个操作标示ID

#if (CUR_PLATFROM == PLATFORM_WIN32)
	typedef int32_t			socklen_t;
#else
	typedef unsigned long 	DWORD;
	#define _MAX_PATH		255
#endif

class CSession;
class CData;
typedef void (CALLBACK CSession::*SERVER_TIMEOUT_CALLBACK)(CData *pData);
typedef void (CALLBACK *NORMAL_TIMEOUT_CALLBACK)(CData *pData);
typedef void (CALLBACK CSession::*SERVER_RESPOND)(SESSION_ID session_id, SERVER_HANDLE source, CData *pData);
typedef void (CALLBACK *NORMAL_RESPOND)(SESSION_ID session_id, SERVER_HANDLE source, CData *pData);

#define SERVER_TIMEOUT_SELECTOR( selector )		static_cast<SERVER_TIMEOUT_CALLBACK>(&selector)
#define NORMAL_TIMEOUT_SELECTOR( selector )		(selector)
#define SERVER_RESPOND_SELECTOR( selector )		static_cast<SERVER_RESPOND>(&selector)
#define NORMAL_RESPOND_SELECTOR( selector )		(selector)

enum : SERVER_HANDLE
{
	INVALID_SERVER_HANDLE = 0,
	GLOBAL_LOG_HANDLE = 1,
	SOCKET_DRIVER_HANDLE = 2,
	//////////////////////////////////////////////////////////////////////////
	//must be last
	USER_SERVER_HANLE_BEGAN = 10,
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

	TSK_SOCKET,
};


#endif//__2015_04_14_COMMON_H__
