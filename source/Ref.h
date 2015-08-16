#ifndef __2015_05_03_REF_H__
#define __2015_05_03_REF_H__

#include <cstdint>

class NoCopyAble
{
private:
	NoCopyAble(const NoCopyAble&);
	NoCopyAble& operator=(const NoCopyAble&);
};

class CRef
{
public:
	CRef();
	CRef(const CRef& rV);
	CRef& operator=(const CRef& rV);
	virtual ~CRef();

	void retain();
	void release();


private:
	int32_t		*m_ref;
};
#endif//__2015_05_03_REF_H__