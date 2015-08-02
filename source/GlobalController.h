/********************************************************************************
*	文件名称:	GlobalController.h												*
*	创建时间：	2015/04/09														*
*	作   者 :	xzben															*
*	文件功能:	系统的全局控制器，用于控制服务器的公共操作							*
*********************************************************************************/

#ifndef __2015_04_09_GLOBAL_CONTROLLER_H__
#define __2015_04_09_GLOBAL_CONTROLLER_H__

#include "SingleInstance.h"
#include "Condition.h"
#include "common.h"
#include "LogServer.h"
#include "ConfigFile.h"
#include <map>

#if (CUR_PLATFROM == PLATFORM_WIN32)
#define __FUNCTION_NAME__	__FUNCTION__
#elif (CUR_PLATFROM == PLATFROM_LINUX )
	#define __FUNCTION_NAME__	__func__
#endif

#define LOG_DETAIL( func ) \
	;//func(GLOBAL_LOG_HANDLE, "[ %s | %d ] >> %s", __FILE__, __LINE__, __FUNCTION_NAME__);

// 全局日志宏
#define LOG_DEBUG(pattern, ...)	\
	{ \
	LOG_DETAIL(CGlobalController::getInstance()->debug); \
	CGlobalController::getInstance()->debug(GLOBAL_LOG_HANDLE, (pattern), __VA_ARGS__); \
	}

#define LOG_INFO(pattern, ...)  \
	{\
	LOG_DETAIL(CGlobalController::getInstance()->info); \
	CGlobalController::getInstance()->info(GLOBAL_LOG_HANDLE, (pattern), __VA_ARGS__); \
	}

#define LOG_WARN(pattern, ...)	\
	{\
	LOG_DETAIL(CGlobalController::getInstance()->warn); \
	CGlobalController::getInstance()->warn(GLOBAL_LOG_HANDLE, (pattern), __VA_ARGS__); \
	}

#define LOG_ERROR(pattern, ...) \
	{ \
	LOG_DETAIL(CGlobalController::getInstance()->error); \
	CGlobalController::getInstance()->error(GLOBAL_LOG_HANDLE, (pattern), __VA_ARGS__); \
	}

#define LOG_FATAL(pattern, ...) \
	{ \
	LOG_DETAIL(CGlobalController::getInstance()->fatal); \
	CGlobalController::getInstance()->fatal(GLOBAL_LOG_HANDLE, (pattern), __VA_ARGS__); \
	}

class CGlobalController : public SingleInstance<CGlobalController>
{
	friend class ThreadGuard;
	friend class SingleInstance<CGlobalController>;
public:
	void			init();
	CConfigFile*	getConfig() { return m_config; }
	bool			isSystemStop(){ return m_bIsSystemStop; }
	void			stopSystem();
	void			waitSystemStop();
	
	void			log(SERVER_HANDLE source, const LOG_LEVEL level, const char* pattern, ...);
	void			debug(SERVER_HANDLE source, const char * pattern, ...);
	void			info(SERVER_HANDLE source, const char * pattern, ...);
	void			warn(SERVER_HANDLE source, const char * pattern, ...);
	void			error(SERVER_HANDLE source, const char * pattern, ...);
	void			fatal(SERVER_HANDLE source, const char * pattern, ...);

	void			timeout(SERVER_HANDLE source, uint32_t delay_msec, SERVER_TIMEOUT_CALLBACK callback, CData *pData = nullptr);
	void			timeout(SERVER_HANDLE source, uint32_t delay_msec, NORMAL_TIMEOUT_CALLBACK callback, CData *pData = nullptr);
protected:
	void			logva(SERVER_HANDLE source, const LOG_LEVEL level, const char* pattern, va_list vp);


	void			addRunThread();
	void			decRunThread();
	CGlobalController();
	virtual ~CGlobalController();
private:
	uint32_t		m_nRunThreadNum;
	bool			m_bIsSystemStop;
	CLogServer*		m_logServer;
	CConfigFile*	m_config;
};

class ThreadGuard
{
public:
	ThreadGuard()
	{
		CGlobalController::getInstance()->addRunThread();
	}
	~ThreadGuard()
	{
		CGlobalController::getInstance()->decRunThread();
	}
};
#endif//__2015_04_09_GLOBAL_CONTROLLER_H__