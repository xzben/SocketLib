#include "LogServer.h"
#include "config.h"
#include "Task.h"
#include <iostream>
#include <cassert>
#include <cstdio>
#include "TimerWorker.h"
#include "GlobalController.h"

CLogServer::CLogServer()
:CServer(SERVER_HANDLE_LOG)
{
	m_fp_console = stdout;
	m_fp_file = nullptr;
	m_day = 0;
	m_file = CGlobalController::getInstance()->getLogFileName();
}

CLogServer::~CLogServer()
{

}

void CLogServer::excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)
{
	assert(type == TaskType::SYS_LOG);
	std::string log = pData->pop<std::string, 0>();

	
	struct tm now = CTimerWorker::getInstance()->getCurTime();
	if (!m_file.empty()) //如果设置了本地日志文件
	{
		if (m_day != now.tm_mday) //如果日志日期（天）变化,则创建一个新的文本文件
		{
			if (NULL != m_fp_file)
			{
				fclose(m_fp_file);
			}
			m_day = now.tm_mday;
			char   szName[_MAX_PATH];
			_snprintf(szName, sizeof(szName), "%s[%04d%02d%02d].log", m_file.c_str(), now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
			m_fp_file = fopen(szName, "at");
		}
	}

	std::string fmtTime = CTimerWorker::getInstance()->getFormatTime();

	if (NULL != m_fp_console)
	{
		fprintf(m_fp_console, "[ %s : 0x%x : %d ] %s\n", fmtTime.c_str(), source, session, log.c_str());
	}

	if (NULL != m_fp_file)
	{
		fprintf(m_fp_file, "[ %s : 0x%x : %d ] %s\n", fmtTime.c_str(), source, session, log.c_str());
	}
}