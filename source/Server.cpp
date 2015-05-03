#include "Server.h"
#include "Atom.h"
#include "ServerManager.h"
#include "WorkerPool.h"
#include "config.h"
#include "WorkerPool.h"
#include "GlobalController.h"
#include "MasterServer.h"
#include "TimerWorker.h"
#include <cstdarg>

SERVER_HANDLE CServer::s_server_handle = SYS_SERVER_HANLE_END;

CServer::CServer(SERVER_HANDLE handle)
: m_bIsInGlobalQueue(false)
, m_serverName("")
{
	if (handle < SYS_SERVER_HANLE_END)
		m_server_handle = handle;
	else
		m_server_handle = AtomSelfAdd(&s_server_handle);

	if (!CServerManager::getInstance()->registerServerByHandle(m_server_handle, this))
	{
		LOG_ERROR("register server by hadle failed by Handle: %d", s_server_handle);
	}
}

CServer::~CServer()
{
	CServerManager::getInstance()->unregisterServer(this);
}

bool CServer::setServerName(std::string name)
{
	if (name.empty()) return false;
	bool ok = CServerManager::getInstance()->registerServerByName(name, this);
	if (ok)
	{
		m_lock.lock();
		m_serverName = name;
		m_lock.unlock();
	}
	return ok;
}

std::string	CServer::getServerName()
{ 
	std::string ret = "";
	
	m_lock.lock();
	ret = m_serverName;
	m_lock.unlock();

	return ret; 
}

bool CServer::addTask(CTask const*pTask)
{
	if (CGlobalController::getInstance()->isSystemStop())
	{
		collectStopTask(pTask);
		return false;
	}
	else
	{
		addTask2WQueue(pTask);
		return true;
	}
}

void CServer::setFlagInGlobalQueue()
{
	AtomSet(&m_bIsInGlobalQueue, 1);
}
void CServer::clearFlagInGlobalQueue()
{
	AtomSet(&m_bIsInGlobalQueue, 0);
}
bool CServer::isInGlobalQueue()
{
	return AtomGet(&m_bIsInGlobalQueue) == 1;
}
int32_t	CServer::dispatchTasks(int num)
{
	QueueContainer queue;
	uint32_t remain_size = this->checkQueue(queue, num);
	int workNum = 0;
	while (!queue.empty())
	{
		CTask const* pTask = queue.front();
		queue.pop();
		workNum++;
		//////////////////////////////////////////////////////////////////////////
		//Ö´ÐÐÈÎÎñ
		if ( !doSessionCall(pTask->m_session, pTask->m_source, pTask->m_data) )
			this->excuteOneTask(pTask->m_type, pTask->m_session, pTask->m_source, pTask->m_data);
		//////////////////////////////////////////////////////////////////////////
		CTask::free(pTask);
	}
	
	if ( getSize() > 0)
	{
		CWorkerPool::getInstance()->addServer(this);
	}
	else
	{
		clearFlagInGlobalQueue();
	}
	
	return workNum;
}

void	CServer::log(const LOG_LEVEL level, const char* pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(level, pattern, vp);
	va_end(vp);
}

void	CServer::debug(const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(LOG_LEVEL::LOG_DEBUG, pattern, vp);
	va_end(vp);
}

void	CServer::error(const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(LOG_LEVEL::LOG_ERROR, pattern, vp);
	va_end(vp);
}

void	CServer::info(const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(LOG_LEVEL::LOG_INFO, pattern, vp);
	va_end(vp);

}

void	CServer::fatal(const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(LOG_LEVEL::LOG_FATAL, pattern, vp);
	va_end(vp);
}

void CServer::warn(const char * pattern, ...)
{
	va_list vp;
	va_start(vp, pattern);
	logva(LOG_LEVEL::LOG_WARN, pattern, vp);
	va_end(vp);
}

void CServer::logva(const LOG_LEVEL level, const char* pattern, va_list vp)
{
	LOG_LEVEL sys_level = CGlobalController::getInstance()->getLogLevel();
	if (sys_level > level) return;

	char buffer[8192];
	int len = vsprintf(buffer, pattern, vp);
	CData *pData = new CData;
	pData->push<char*>(buffer);
	CWorkerPool::getInstance()->addTask(SERVER_HANDLE_LOG, CTask::create(TaskType::SYS_LOG, 0, getServerHandle(), pData));
	pData->release();
}

void CServer::timeout(uint32_t delay_msec, SERVER_TIMEOUT_CALLBACK callback, CData *pData /*=nullptr*/)
{
	SESSION_ID session_id = newTimeOutSession(callback);
	CTimerWorker::getInstance()->addTimeout(delay_msec, m_server_handle, session_id);

}

void CServer::timeout(uint32_t delay_msec, NORMAL_TIMEOUT_CALLBACK callback, CData *pData /*=nullptr*/)
{
	SESSION_ID session_id = newTimeOutSession(callback);
	CTimerWorker::getInstance()->addTimeout(delay_msec, m_server_handle, session_id);
}

void CServer::collectStopTask(CTask const *pTask)
{
	CTask::free(pTask);
}