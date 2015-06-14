/********************************************************************************
*	文件名称:	MasterServer.h													*
*	创建时间：	2015/04/11														*
*	作   者 :	xzben															*
*	文件功能:	系统的中心服务													*
*********************************************************************************/
#ifndef __2015_04_11_MASTER_SERVER_H__
#define __2015_04_11_MASTER_SERVER_H__

#include "Server.h"
#include "SingleInstance.h"

class CMasterServer : public CServer, public SingleInstance<CMasterServer>
{
public:

protected:
	friend class SingleInstance<CMasterServer>;
	CMasterServer();
	virtual ~CMasterServer();
	virtual void initInstance() override;
	virtual void excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)override;
protected:
	void CALLBACK tick(CData *pData);
private:
	int		m_count;
};
#endif//__2015_04_11_MASTER_SERVER_H__