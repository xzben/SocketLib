/********************************************************************************
*	文件名称:	DTQueue.h														*
*	创建时间：	2014/03/02														*
*	作   者 :	xzben															*
*	文件功能:	一个线程安全的读写双队列，减少竞争导致的锁消耗						*
*********************************************************************************/
#ifndef __2014_03_02_CTASK_H__
#define __2014_03_02_CTASK_H__


#include <mutex>
#include <queue>
#include <cstdint>

template<typename	_TaskType>
class DTQueue
{
public:
	DTQueue() :m_nSize(0){}
	virtual ~DTQueue(){}

	void	addTask2WQueue(const _TaskType& pTask)
	{
		m_wlock.lock();
		m_nSize++;
		m_WriteQueue.push(pTask);
		m_wlock.unlock();
	}

	// 尝试取出指定数量的任务，并返回剩余的任务
	int32_t	checkQueue(std::queue<_TaskType>& getQueue, int nNum)
	{
		int32_t bRet = 0;
		m_wlock.lock();
		if (m_nSize > 0)
		{
			int readNum = 0;
			while (!m_WriteQueue.empty() && (nNum > readNum || nNum < 0))
			{
				_TaskType& task = m_WriteQueue.front();
				getQueue.push(task);
				m_WriteQueue.pop();
				readNum++;
				m_nSize--;
				bRet = true;
			}
		}
		bRet = m_nSize;
		
		m_wlock.unlock();

		return bRet;
	}

	uint32_t  getSize()
	{
		uint32_t ret = 0;
		
		m_wlock.lock();
		ret = m_nSize;
		m_wlock.unlock();

		return ret;
	}

private:
	DTQueue& operator =(const DTQueue& r);
	DTQueue(const DTQueue& r);
protected:
	uint32_t						m_nSize;
	std::mutex						m_wlock;
	typedef std::queue<_TaskType>	QueueContainer;
	QueueContainer					m_WriteQueue;
};


#endif//__2014_03_02_CTASK_H__