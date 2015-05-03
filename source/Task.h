/********************************************************************************
*	�ļ�����:	Task.h															*
*	����ʱ�䣺	2014/06/24														*
*	��   �� :	xzben															*
*	�ļ�����:	����ִ�е�����													*
*********************************************************************************/
#ifndef __2015_03_29_TASK_H__
#define __2015_03_29_TASK_H__

#include "common.h"
#include <cstdint>
#include <string>
#include <vector>
#include "Atom.h"

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


class CData
{
public:
	CData()
	{
		m_ref = new int(1);
	}

	void retain()
	{
		AtomSelfAdd(m_ref);
	}

	void release()
	{
		if (AtomSelfDec(m_ref) <= 0)
		{
			delete this;
		}
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

	template<>
	void push<char*>(char* str)
	{
		m_datas.push_back(new DataBlock<std::string>(str));
		//this->push(new DataBlock<std::string>(str));
	}

	template<>
	void push<const char*>(const char* str)
	{
		m_datas.push_back(new DataBlock<std::string>(str));
		//this->push(new DataBlock<std::string>(str));
	}

	template<typename Type, int index>
	Type	pop()
	{
		Type ret;
		
		if (index < getSize())
		{
			DataBlockBase* tmp = m_datas[index];
			DataBlock<Type> *ptr = dynamic_cast<DataBlock<Type>*>(tmp);

			if (ptr != nullptr)
			{
				return ptr->getData();
			}
		}
		else
		{
			socketlib_error("try to pop an empty CData!!:" __FUNCDNAME__);
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
	//����char* ����
	template<>
	char* pop<char*, 0>();

	template<>
	const char* pop<const char*, 0>();
	~CData()
	{
		clear();
		SAFE_DELETE(m_ref);
	}
	std::vector<DataBlockBase*>	m_datas;
	int *m_ref;
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