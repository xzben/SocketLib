#include "GlobalController.h"
#include "WorkerPool.h"
#include "ServerManager.h"
#include "TimerWorker.h"
#include "Atom.h"
#include <cstdarg>
#include <iostream>
#include <thread>
#include <mutex>

CGlobalController::CGlobalController()
{
	m_bIsSystemStop = false;
	m_nRunThreadNum = 0;
	m_config = new CConfigFile;
}

CGlobalController::~CGlobalController()
{
	SAFE_DELETE(m_config);
	SAFE_DELETE(m_logServer);
}

void CGlobalController::init()
{
	m_logServer = new CLogServer;
}

void CGlobalController::stopSystem()
{
	m_bIsSystemStop = true;
}

void CGlobalController::waitSystemStop()
{
	while (AtomGet(&m_nRunThreadNum) > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void	CGlobalController::log(SERVER_HANDLE source, const LOG_LEVEL level, const char* pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(source, level, pattern, vp);
	va_end(vp);
}

void	CGlobalController::debug(SERVER_HANDLE source, const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(source, LOG_LEVEL::LOG_DEBUG, pattern, vp);
	va_end(vp);
}

void	CGlobalController::error(SERVER_HANDLE source, const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(source, LOG_LEVEL::LOG_ERROR, pattern, vp);
	va_end(vp);
}

void	CGlobalController::info(SERVER_HANDLE source, const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(source, LOG_LEVEL::LOG_INFO, pattern, vp);
	va_end(vp);

}

void	CGlobalController::fatal(SERVER_HANDLE source, const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(source, LOG_LEVEL::LOG_FATAL, pattern, vp);
	va_end(vp);
}

void CGlobalController::warn(SERVER_HANDLE source, const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(source, LOG_LEVEL::LOG_WARN, pattern, vp);
	va_end(vp);
}

void CGlobalController::logva(SERVER_HANDLE source, const LOG_LEVEL level, const char* pattern, va_list vp)
{
	LOG_LEVEL sys_level = m_logServer->getLevel();
	if (sys_level > level) return;

	char buffer[8192];
	int len = vsprintf(buffer, pattern, vp);
	CData *pData = new CData;
	pData->push<char*>(buffer);
	m_logServer->addTask( CTask::create(TaskType::SYS_LOG, 0, source, pData) );

	pData->release();
}

void CGlobalController::timeout(SERVER_HANDLE source, uint32_t delay_msec, SERVER_TIMEOUT_CALLBACK callback, CData *pData /*= nullptr*/)
{
	CServer *server = CServerManager::getInstance()->getServerByHandle(source);
	if (server != nullptr) server->timeout(delay_msec, callback, pData);
}

void CGlobalController::timeout(SERVER_HANDLE source, uint32_t delay_msec, NORMAL_TIMEOUT_CALLBACK callback, CData *pData /*= nullptr*/)
{
	CServer *server = CServerManager::getInstance()->getServerByHandle(source);
	if (server != nullptr)	server->timeout(delay_msec, callback, pData);
}

void CGlobalController::addRunThread()
{
	AtomSelfAdd(&m_nRunThreadNum); 
}

void CGlobalController::decRunThread()
{ 
	AtomSelfDec(&m_nRunThreadNum); 
}