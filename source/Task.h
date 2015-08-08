/********************************************************************************
*	文件名称:	Task.h															*
*	创建时间：	2014/06/24														*
*	作   者 :	xzben															*
*	文件功能:	服务执行的任务													*
*********************************************************************************/
#ifndef __2015_03_29_TASK_H__
#define __2015_03_29_TASK_H__

#include "common.h"
#include <cstdint>
#include <string>
#include <vector>
#include "Ref.h"

class DataBlockBase
{
public:
	DataBlockBase(){};
	virtual ~DataBlockBase(){};
};

template<typename Type>
class DataBlock : public DataBlockBase
{
public:
	typedef	Type	DataType;

	DataBlock(DataType data)
	{
		m_data = data;
	}
	virtual ~DataBlock()
	{

	}

	void	 setData(DataType data) { m_data = data; }
	DataType getData() { return m_data; }
private:
	DataType	m_data;
};


class CData : public CRef
{
public:
	CData()
	{
		
	}
	virtual ~CData()
	{
		clear();
	}

	inline int getSize()
	{
		return m_datas.size();
	}

	template<typename Type>
	void push(Type data)
	{
		m_datas.push_back(new DataBlock<Type>(data));
	}

	template<typename Type, int index>
	Type	pop(Type defValue)
	{
		Type ret = defValue;
		
		if (index < getSize())
		{
			DataBlockBase* tmp = m_datas[index];
			DataBlock<Type> *ptr = dynamic_cast<DataBlock<Type>*>(tmp);

			if (ptr != nullptr)
			{
				ret = ptr->getData();
			}
		}
		else
		{
			socketlib_error("try to pop an empty CData!!: %s", __func__);
		}
		return ret;
	}
private:
	void clear()
	{
		auto it = m_datas.begin();
		while (it != m_datas.end())
		{
			DataBlockBase*& tmp = *it;
			SAFE_DELETE(tmp);
			it++;
		}
		m_datas.clear();
	}
	std::vector<DataBlockBase*>	m_datas;
};
class CTask
{
public:
	CTask(const CTask& rV);
	CTask& operator = (const CTask& rv);

	static CTask* create(TaskType type, SESSION_ID session_id, SERVER_HANDLE source, CData *m_data);
	static void   free(CTask const*& pTask);

	TaskType		m_type;
	SESSION_ID		m_session;
	SERVER_HANDLE	m_source;
	CData		    *m_data;
protected:
	CTask();
	~CTask();
};
#endif//__2015_03_29_TASK_H__
