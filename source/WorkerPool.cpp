#include "WorkerPool.h"
#include "Thread.h"
#include "Condition.h"
#include "ServerManager.h"
#include "GlobalController.h"
#include <queue>
#include <iostream>
#include <cassert>
#include <mutex>
#include "DTQueue.h"
#include "MasterServer.h"

class WorkThreadPool : public DTQueue<CServer*>
{
public:
	explicit WorkThreadPool(uint32_t nThreadNum);
	virtual ~WorkThreadPool();
	
	void start();
	bool addTask(SERVER_HANDLE server_handle, CTask* pTask);
	void addServer(CServer *&pServer);

protected:
	void run(uint32_t id);
	class WorkThread : public CThread
	{
	public:
		WorkThread(){ m_pParentPool = nullptr; }
		WorkThread(WorkThreadPool* pParentPool):m_pParentPool(pParentPool){}
		~WorkThread(){}

		void	setThreadID(uint32_t id){ m_nID = id; }
		void	setParentPool(WorkThreadPool* pParentPool){ m_pParentPool = pParentPool; }
		virtual void run() 
		{ 
			assert(m_pParentPool != nullptr); 
			m_pParentPool->run(m_nID); 
		}
	protected:
		WorkThreadPool* m_pParentPool;
		uint32_t		m_nID;
	};
	WorkThread				*m_pWorkers;
	CCondition				m_signalStart;
	CCondition				m_signalNewTask;
	uint32_t				m_nWorksNum;
};

WorkThreadPool::WorkThreadPool(uint32_t nThreadNum)
{
	m_nWorksNum = nThreadNum;
	m_pWorkers = new WorkThreadPool::WorkThread[nThreadNum];
	for (uint32_t i = 0; i < nThreadNum; i++)
	{
		m_pWorkers[i].setThreadID(i + 1);
		m_pWorkers[i].setParentPool(this);
		m_pWorkers[i].start();
	}
}

WorkThreadPool::~WorkThreadPool()
{
	delete[] m_pWorkers;
}

void WorkThreadPool::run(uint32_t id)
{
	ThreadGuard guard;
	m_signalStart.wait();
	WorkThreadPool::QueueContainer container;
	const uint32_t EXCUTE_TASK_NUM_PER_SERVER = 10;
	const uint32_t EXCUTE_SERVER_NUM = 1;

	while (!CGlobalController::getInstance()->isSystemStop())
	{
		m_signalNewTask.wait();
		this->checkQueue(container, EXCUTE_SERVER_NUM);
		while (!container.empty())
		{
			while (!container.empty())
			{
				CServer* pServer = container.front();
				container.pop();
				pServer->dispatchTasks(EXCUTE_TASK_NUM_PER_SERVER);

				//服务器标记结束了，且服务的任务执行完了则结束任务
				if (CGlobalController::getInstance()->isSystemStop() && pServer->getSize() <= 0)
				{
					pServer->release();
				}
			}
			this->checkQueue(container, EXCUTE_SERVER_NUM);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void WorkThreadPool::start()
{
	m_signalStart.notify_all();
}

void WorkThreadPool::addServer(CServer *&pServer)
{
	pServer->setFlagInGlobalQueue();
	addTask2WQueue(pServer);
	m_signalNewTask.notify_all();
}

bool WorkThreadPool::addTask(SERVER_HANDLE server_handle, CTask* pTask)
{
	CServer* pServer = CServerManager::getInstance()->getServerByHandle(server_handle);
	if (nullptr == pServer)
	{
		LOG_ERROR(" can't find Server by Handle: %d", server_handle);
		return false;
	}
	
	pServer->addTask(pTask);
	m_signalNewTask.notify_all();
	return true;
}
//////////////////////////////////////////////////////////////////////////
CWorkerPool::CWorkerPool()
{
	uint32_t nWorkerNum = CGlobalController::getInstance()->getConfig()->getInt("workThread", 8);
	m_pCore = new WorkThreadPool(nWorkerNum);
}

CWorkerPool::~CWorkerPool()
{

}

bool  CWorkerPool::addTask(SERVER_HANDLE server_handle, CTask* pTask)
{
	return m_pCore->addTask(server_handle, pTask);
}

void  CWorkerPool::addServer(CServer* pServer)
{
	return m_pCore->addServer(pServer);
}
void  CWorkerPool::start()
{
	return m_pCore->start();
}