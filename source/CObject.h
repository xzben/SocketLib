#ifndef __2014_03_02_COBJECT_H__
#define __2014_03_02_COBJECT_H__

#include <assert.h>
#include "config.h"
#include "COS.h"


/*
*	私有化复制构造函数和复制函数
*	使子类对象无法复制
*/
class NoCopyable
{
protected:
	NoCopyable(){};
	~NoCopyable(){};
private:
	NoCopyable(const NoCopyable& );
	NoCopyable& operator=(const NoCopyable&);
};

class CObject
{
public:
	CObject();
	virtual ~CObject();
	
private:
	int		m_nObjectId;
	int		m_nRefence;
};

#endif//__2014_03_02_COBJECT_H__