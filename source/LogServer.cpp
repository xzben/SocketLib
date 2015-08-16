#include "LogServer.h"
#include "config.h"
#include "Task.h"
#include <iostream>
#include <cassert>
#include <cstdio>
#include "TimerWorker.h"
#include "GlobalController.h"


CLogServer::CLogServer()
{
	m_fp_console = stdout;
	m_fp_file = NULL;
	m_day = 0;
	m_file  = CGlobalController::getInstance()->getConfig()->getString("logFile", "");
	m_level = getLevelByString(CGlobalController::getInstance()->getConfig()->getString("logLevel", "off"));
}

CLogServer::~CLogServer()
{
	if (NULL != m_fp_file)
	{
		fflush(m_fp_file);
		fclose(m_fp_file);
	}
}

LOG_LEVEL CLogServer::getLevel()
{
	return m_level;
}

LOG_LEVEL CLogServer::getLevelByString(std::string level)
{
	if (level == "all") return LOG_ALL;
	else if (level == "debug") return LOG_DEBUG;
	else if (level == "info") return LOG_INFO;
	else if (level == "warn") return LOG_WARN;
	else if (level == "error") return LOG_ERROR;
	else if (level == "fatal") return LOG_FATAL;
	else if (level == "off") return LOG_OFF;

	return LOG_OFF;
}


void CLogServer::excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)
{
	assert(type == TaskType::SYS_LOG);
	std::string log = pData->pop<std::string>(0, "");

	
	struct tm now = CTimerWorker::getInstance()->getCurTime();
	if (!m_file.empty()) //如果设置了本地日志文件
	{
		if (m_day != now.tm_mday) //如果日志日期（天）变化,则创建一个新的文本文件
		{
			if (NULL != m_fp_file)
			{
				fflush(m_fp_file);
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
