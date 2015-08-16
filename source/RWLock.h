#ifndef __2015_08_09_RWLOCK_H__
#define __2015_08_09_RWLOCK_H__

#include <cstdint>

class RWLcok
{
public:
	RWLcok();

	void  rLock();
	void  rUnlock();

	void  wLock();
	void  wUnlock();
private:
	uint32_t		   m_rCnt;
	uint32_t		   m_wCnt;
};
#endif//__2015_08_09_RWLOCK_H__