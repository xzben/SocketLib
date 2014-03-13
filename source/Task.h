#ifndef __2014_03_02_CTASK_H__
#define __2014_03_02_CTASK_H__


#include "CObject.h"

/**
*	任务队列模板，使用的是双队列的思想，主要是为了减少互斥锁的使用
*	在模板中已经存在队列一了，此队列用于任务队列实现多线程安全添加任务的存储。
*	队列二是用于存储具体使用时候存储。
*		要由具体使用任务队列的子类实现，并实现 _addTask() 接口，将存储的任务放入使用队列中
**/
template<typename	_TaskType>
class TaskQueue : public NoCopyable
{
public:
	TaskQueue():m_nSize(0){}
	virtual ~TaskQueue(){}

	void	addTask(const _TaskType& pTask)
	{
		AutoLock lock(&m_TaskQueuemutex);
		m_taskQueue.push(pTask);
		m_nSize ++;
	}

	bool	checkQueue()
	{
		AutoLock lock(&m_TaskQueuemutex);
		while(!m_taskQueue.empty())
		{
			_TaskType& task = m_taskQueue.front();
			_addTask(task);
			m_taskQueue.pop();
		}
		m_nSize = 0;
		return true;
	}
protected:
	virtual void	_addTask(_TaskType pTask) = 0;
	int				m_nSize;
protected:
	Mutex				m_TaskQueuemutex;
	typedef std::queue<_TaskType>	QueueContainer;
	QueueContainer		m_taskQueue;
};


#endif//__2014_03_02_CTASK_H__