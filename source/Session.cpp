#include "Session.h"
#include "Atom.h"

CSession::CSession()
{
	AtomSet(&m_session_count, 0);
}

CSession::~CSession()
{

}

SESSION_ID CSession::newSession()
{
	return AtomSelfAdd(&m_session_count);
}

SESSION_ID CSession::newTimeOutSession(SERVER_TIMEOUT_CALLBACK callback)
{
	SESSION_ID session_id = newSession();
	m_timeoutRespond.insert(RCALL_MAP::value_type(session_id, RCALL(callback)));

	return session_id;
}

SESSION_ID CSession::newTimeOutSession(NORMAL_TIMEOUT_CALLBACK callback)
{
	SESSION_ID session_id = newSession();
	m_timeoutRespond.insert(RCALL_MAP::value_type(session_id, RCALL(callback)));

	return session_id;
}

SESSION_ID CSession::newRPCSession(SERVER_RESPOND callback)
{
	SESSION_ID session_id = newSession();
	m_timeoutRespond.insert(RCALL_MAP::value_type(session_id, RCALL(callback)));

	return session_id;
}

SESSION_ID CSession::newRPCSession(NORMAL_RESPOND callback)
{
	SESSION_ID session_id = newSession();
	m_timeoutRespond.insert(RCALL_MAP::value_type(session_id, RCALL(callback)));

	return session_id;
}

bool CSession::doSessionCall(SESSION_ID session_id, SERVER_HANDLE source, CData *pData)
{
	auto it = m_timeoutRespond.find(session_id);
	if (it == m_timeoutRespond.end())
		return false;

	RCALL call = it->second;
	switch (call.type)
	{
	case STCALL:
		(this->*(call.stcall))(pData);
		break;
	case NTCALL:
		call.ntcall(pData);
		break;
	case SRPC:
		(this->*call.srpc)(session_id, source, pData);
		break;
	case NRPC:
		call.nrpc(session_id, source, pData);
		break;
	default:
		return false;
	}

	m_timeoutRespond.erase(it);
	return true;
}
