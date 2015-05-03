#include "ServerManager.h"
#include "Atom.h"

CServerManager::CServerManager()
{
	AtomSet(&m_curServerCount, 0);
}

CServerManager::~CServerManager()
{

}

void CServerManager::clearServer(CServer* pServer)
{
	if (pServer->getServerHandle() >= SYS_SERVER_HANLE_END)
	{
		SAFE_DELETE(pServer);
	}
}

uint32_t CServerManager::getCurServerNum()
{
	return AtomGet(&m_curServerCount);
}

CServer* CServerManager::getServerByHandle(SERVER_HANDLE server_handle)
{
	CServer *pRet = nullptr;

	m_lockHandle.lock();
	auto it = m_handle2Server.find(server_handle);
	if (it != m_handle2Server.end())
	{
		pRet = it->second;
	}
	m_lockHandle.unlock();

	return pRet;
}

CServer* CServerManager::getServerByName(std::string server_name)
{
	CServer *pRet = nullptr;

	m_lockName.lock();
	auto it = m_name2Server.find(server_name);
	if (it != m_name2Server.end())
	{
		pRet = it->second;
	}
	m_lockName.unlock();

	return pRet;
}

bool CServerManager::registerServerByHandle(SERVER_HANDLE server_handle, CServer *pserver)
{
	bool bRet = true;

	m_lockHandle.lock();
	auto it = m_handle2Server.find(server_handle);
	if (it != m_handle2Server.end())
		bRet = false;
	else
	{
		m_handle2Server.insert(ConHandle2Server::value_type(server_handle, pserver));
	}
	m_lockHandle.unlock();
	if (bRet)
		AtomSelfAdd(&m_curServerCount);

	return bRet;
}
bool CServerManager::registerServerByName(std::string server_name, CServer *pserver)
{
	bool bRet = true;

	m_lockName.lock();
	auto it = m_name2Server.find(server_name);
	if (it != m_name2Server.end())
		bRet = false;
	else
	{
		m_name2Server.insert(ConName2Server::value_type(server_name, pserver));
	}
	m_lockName.unlock();

	return bRet;
}

void CServerManager::unregisterServerByName(std::string server_name)
{
	if (server_name.empty()) return;

	m_lockName.lock();
	auto it = m_name2Server.find(server_name);
	if (it != m_name2Server.end())
		m_name2Server.erase(it);
	m_lockName.unlock();
}

void CServerManager::unregisterServerByHandle(SERVER_HANDLE server_handle)
{
	bool ok = false;
	m_lockHandle.lock();
	auto it = m_handle2Server.find(server_handle);
	if (it != m_handle2Server.end())
	{
		m_handle2Server.erase(it);
		ok = true;
	}
	m_lockHandle.unlock();

	if (ok)
		AtomSelfDec(&m_curServerCount);
}
void CServerManager::unregisterServer(CServer *pServer)
{
	SERVER_HANDLE server_handle = pServer->getServerHandle();
	std::string   server_name = pServer->getServerName();

	unregisterServerByHandle(server_handle);
	unregisterServerByName(server_name);
}