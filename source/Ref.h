#ifndef __2015_05_03_REF_H__
#define __2015_05_03_REF_H__

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
	int		*m_ref;
};
#endif//__2015_05_03_REF_H__