/********************************************************************************
*	文件名称:	SingleInstance.h												*
*	创建时间：	2015/04/09														*
*	作   者 :	xzben															*
*	文件功能:	单例模式的模板基类														*
*********************************************************************************/
#ifndef __2015_04_09_SINGLE_INSTANCE_H__
#define __2015_04_09_SINGLE_INSTANCE_H__

template<typename TClass>
class SingleInstance
{
public:
	static TClass* getInstance()
	{
		if (getSingleObj() == nullptr)
		{
			getSingleObj() = new TClass();
			getSingleObj()->initInstance();
		}

		return getSingleObj();
	}
	virtual void initInstance(){}
protected:
	friend class SingleInstanceManager;
	SingleInstance(){};
	virtual ~SingleInstance(){};

	static TClass*& getSingleObj()
	{
		static TClass* s_instance = nullptr;
		return s_instance;
	}
};
#endif//__2015_04_09_SINGLE_INSTANCE_H__