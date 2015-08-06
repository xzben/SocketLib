#include "Server.h"
#include "Atom.h"
#include "ServerManager.h"
#include "WorkerPool.h"
#include "config.h"
#include "WorkerPool.h"
#include "GlobalController.h"
#include "MasterServer.h"
#include "TimerWorker.h"

SERVER_HANDLE CServer::s_server_handle = USER_SERVER_HANLE_BEGAN;

CServer::CServer()
: m_bIsInGlobalQueue(false)
, m_serverName("")
{
	m_server_handle = AtomSelfAdd(&s_server_handle);
	CServerManager::getInstance()->registerServer(this);
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
		
		if (!isInGlobalQueue())
			CWorkerPool::getInstance()->addServer(this);

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
		//执行任务
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

void CServer::collectStopTask(CTask const *pTask)
{
	CTask::free(pTask);
}

void	CServer::timeout( uint32_t delay_msec, SERVER_TIMEOUT_CALLBACK callback, CData *pData /*= nullptr*/)
{
	SESSION_ID session_id = newTimeOutSession(callback);
	CTimerWorker::getInstance()->addTimeout(delay_msec, m_server_handle, session_id);
}

void	CServer::timeout(uint32_t delay_msec, NORMAL_TIMEOUT_CALLBACK callback, CData *pData /*= nullptr*/)
{
	SESSION_ID session_id = newTimeOutSession(callback);
	CTimerWorker::getInstance()->addTimeout(delay_msec, m_server_handle, session_id);
}