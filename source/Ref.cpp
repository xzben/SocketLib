#include "Ref.h"
#include "Atom.h"
#include "common.h"

CRef::CRef()
{
	m_ref = new int32_t(1);
}

CRef::~CRef()
{
	SAFE_DELETE(m_ref);
}

CRef::CRef(const CRef& rV)
{
	m_ref = rV.m_ref;
	retain();
}

CRef& CRef::operator = (const CRef& rV)
{
	m_ref = rV.m_ref;
	retain();

	return *this;
}

void CRef::retain()
{
	AtomSelfAdd(m_ref);
}

void CRef::release()
{
	if (AtomSelfDec(m_ref) <= 0)
	{
		delete this;
	}
}
