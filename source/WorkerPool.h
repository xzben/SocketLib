/********************************************************************************
*	文件名称:	WorkerPool.h													*
*	创建时间：	2015/03/29														*
*	作   者 :	xzben															*
*	文件功能:	系统的工作池所有的服务器任务的执行都将在其中运行					*
*********************************************************************************/
#ifndef __2015_03_29_WORKER_POOL_H__
#define __2015_03_29_WORKER_POOL_H__

#include <cstdint>
#include "Task.h"
#include "Server.h"
#include "SingleInstance.h"

class WorkThreadPool;

class CWorkerPool : public SingleInstance<CWorkerPool>
{
public:
	bool  addTask(SERVER_HANDLE server_handle, CTask* pTask);
	void  addServer(CServer* pServer);
	void  start();
protected:
	friend class SingleInstance<CWorkerPool>;
	CWorkerPool();
	~CWorkerPool();
private:
	WorkThreadPool* m_pCore;
};
#endif//__2015_03_29_WORKER_POOL_H__