#include "SingleInstance.h"
#include "common.h"

SingleInstanceManager::SingleInstanceManager()
{
	m_allSingles.reserve(10);
}

SingleInstanceManager::~SingleInstanceManager()
{
	clear();
}

void  SingleInstanceManager::clear()
{
	auto it = m_allSingles.begin();
	while (it != m_allSingles.end())
	{
		SingleBase* obj = *it;
		SAFE_DELETE(*it);
		obj->getSingleObj() = nullptr;
	}
	m_allSingles.clear();
}

void  SingleInstanceManager::addSingleObj(SingleBase* obj)
{
	m_allSingles.push_back(obj);
}

SingleInstanceManager g_singleMng;