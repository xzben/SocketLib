#include "RWLock.h"
#include "Atom.h"

RWLcok::RWLcok()
: m_rCnt(0)
, m_wCnt(0)
{

}

void RWLcok::rLock()
{
	while( true )
	{
		while (AtomGet(&m_wCnt) > 0);
		AtomSelfAdd(&m_rCnt);

		if (AtomGet(&m_wCnt) > 0)
		{
			AtomSelfDec(&m_rCnt);
		}
		else
		{
			break;
		}
	}
}

void RWLcok::rUnlock()
{
	AtomSelfDec(&m_rCnt);
}

void RWLcok::wLock()
{
	while (AtomSet(&m_wCnt, 1));
	
	while (AtomGet(&m_rCnt) > 0);

}

void RWLcok::wUnlock()
{
	AtomSet(&m_wCnt, 0);
}