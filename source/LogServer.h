/********************************************************************************
*	文件名称:	LogServer.h														*
*	创建时间：	2015/04/10														*
*	作   者 :	xzben															*
*	文件功能:	日志服务的实现													*
*********************************************************************************/
#ifndef __2015_04_10_LOG_SERVER_H__
#define __2015_04_10_LOG_SERVER_H__

#include "Server.h"
#include "SingleInstance.h"
#include <string>

class CLogServer : public CServer
{
public:
	CLogServer();
	virtual ~CLogServer();

	LOG_LEVEL getLevelByString(std::string level);
	LOG_LEVEL getLevel();
protected:
	virtual void excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)override;
private:
	FILE			*m_fp_console;
	FILE			*m_fp_file;
	int				m_day;			//当前打开日志的是哪天的，每天一个日志文本
	std::string		m_file;
	LOG_LEVEL		m_level;
};
#endif//__2015_04_10_LOG_SERVER_H__