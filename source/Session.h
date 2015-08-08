/********************************************************************************
*	文件名称:	Session.h														*
*	创建时间：	2015/04/12														*
*	作   者 :	xzben															*
*	文件功能:	服务操作Session的管理中心											*
*********************************************************************************/
#ifndef __2015_04_12_SESSION_H__
#define __2015_04_12_SESSION_H__

#include <cstdint>
#include <unordered_map>
#include "common.h"
#include "Task.h"

class CSession
{
	enum RCALL_TYPE: uint8_t
	{
		STCALL,	//SERVER_TIMEOUT_CALLBACK
		NTCALL, //NORMAL_TIMEOUT_CALLBACK
		SRPC,	//SERVER_RESPOND
		NRPC,	//NORMAL_RESPOND
	};
	struct RCALL
	{
		RCALL_TYPE	type;
		union{
			SERVER_TIMEOUT_CALLBACK stcall;
			NORMAL_TIMEOUT_CALLBACK	ntcall;
			SERVER_RESPOND			srpc;
			NORMAL_RESPOND			nrpc;
		};
		RCALL(SERVER_TIMEOUT_CALLBACK func)
		{
			type = STCALL;
			stcall = func;
		}
		RCALL(NORMAL_TIMEOUT_CALLBACK func)
		{
			type = NTCALL;
			ntcall = func;
		}
		RCALL(SERVER_RESPOND func)
		{
			type = NTCALL;
			srpc = func;
		}
		RCALL(NORMAL_RESPOND func)
		{
			type = NTCALL;
			nrpc = func;
		}
	};
public:
	CSession();
	virtual ~CSession();

	SESSION_ID newSession();
	SESSION_ID newTimeOutSession(SERVER_TIMEOUT_CALLBACK callback);
	SESSION_ID newTimeOutSession(NORMAL_TIMEOUT_CALLBACK callback);
	SESSION_ID newRPCSession(SERVER_RESPOND callback);
	SESSION_ID newRPCSession(NORMAL_RESPOND callback);
	bool	   doSessionCall(SESSION_ID session_id, SERVER_HANDLE handle, CData *pData);
private:
	SESSION_ID	m_session_count;
	
	typedef std::unordered_map<SESSION_ID, RCALL> RCALL_MAP;
	RCALL_MAP	m_timeoutRespond;

};
#endif//__2015_04_12_SESSION_H__