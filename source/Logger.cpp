#include "Logger.h"

//////////////////////////////////////////////////////////////////////////
//Logger
Logger::Logger(std::string strLogName /* = "LOGGER" */, LOG_LEVEL emLevel/* = LEVEL_ERROR */)
	:m_strLogName(strLogName), m_emLevel(emLevel)
{
	m_fp_console = stdout;
	m_fp_file = NULL;

	m_file = "";
	m_day = 0;
}

Logger::~Logger()
{
	if(NULL != m_fp_file)
	{
		::fclose(m_fp_file);
		m_fp_file = NULL;
	}
}
void Logger::removeConsoleLog()
{
	m_mutex.lock();
	m_fp_console = NULL;
	m_mutex.unlock();
}

void Logger::addLocalFileLog(const std::string &file)
{
	m_mutex.lock();
	m_day = 0;
	m_file = file;
	m_mutex.unlock();
}
void Logger::setLevel(const LOG_LEVEL level)
{
	m_mutex.lock();
	m_emLevel = level;
	m_mutex.unlock();
}

void Logger::setLevel(const std::string &level)
{
	if ("off" == level) setLevel(LEVEL_OFF);
	else if ("fatal" == level) setLevel(LEVEL_FATAL);
	else if ("error" == level) setLevel(LEVEL_ERROR);
	else if ("warn" == level) setLevel(LEVEL_WARN);
	else if ("info" == level) setLevel(LEVEL_INFO);
	else if ("debug" == level) setLevel(LEVEL_DEBUG);
	else if ("all" == level) setLevel(LEVEL_ALL);
}


void Logger::logtext(const LOG_LEVEL level,const char * text)
{
	if (m_emLevel > level) return;
	log(level,"%s",text);  
}

void Logger::logva(const LOG_LEVEL level,const char * pattern,va_list vp)
{
	SYSTEMTIME system;
	struct tm *now;
	time_t ltime;
	char   szName[_MAX_PATH];

	if (m_emLevel > level) return;
	time(&ltime);
	if (NULL == (now=localtime(&ltime))) return;

	GetLocalTime(&system);

	m_mutex.lock();

	if (!m_file.empty()) //如果设置了本地日志文件
	{
		if (m_day != now->tm_mday) //如果日志日期（天）变化,则创建一个新的文本文件
		{
			if (NULL != m_fp_file)
			{
				fclose(m_fp_file);
			}
			m_day = now->tm_mday;
			_snprintf(szName,sizeof(szName),"%s%04d%02d%02d.log",m_file.c_str(),now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
			m_fp_file = fopen(szName,"at");
		}
	}                 

	if (NULL != m_fp_console)
	{
		fprintf(m_fp_console,"[%s] ",m_strLogName.c_str());
	}
	if (NULL != m_fp_file)
	{
		fprintf(m_fp_file,"[%s] ",m_strLogName.c_str());
	}

	if (NULL != m_fp_console)
	{
		fprintf(m_fp_console,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(m_fp_console,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}
	if (NULL != m_fp_file)
	{
		fprintf(m_fp_file,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(m_fp_file,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}

	if (NULL != m_fp_console)
	{
		vfprintf(m_fp_console,pattern,vp);
		fprintf(m_fp_console,"\n");
		fflush(m_fp_console);
	}
	if (NULL != m_fp_file)
	{
		vfprintf(m_fp_file,pattern,vp);
		fprintf(m_fp_file,"\n");
		fflush(m_fp_file);
	}

	m_mutex.unlock();
}

void Logger::log(const LOG_LEVEL level,const char * pattern,...)
{
	va_list vp;

	if (m_emLevel > level) return;
	va_start(vp,pattern);
	logva(level,pattern,vp);
	va_end(vp);
}

void Logger::fatal(const char * pattern,...)
{
	va_list vp;

	if (m_emLevel > LEVEL_FATAL) return;
	va_start(vp,pattern);
	logva(LEVEL_FATAL,pattern,vp);
	va_end(vp);
}

void Logger::error(const char * pattern,...)
{
	va_list vp;

	if (m_emLevel > LEVEL_ERROR) return;
	va_start(vp,pattern);
	logva(LEVEL_ERROR,pattern,vp);
	va_end(vp);
}

void Logger::warn(const char * pattern,...)
{
	va_list vp;

	if (m_emLevel > LEVEL_WARN) return;
	va_start(vp,pattern);
	logva(LEVEL_WARN,pattern,vp);
	va_end(vp);
}

void Logger::info(const char * pattern,...)
{
	va_list vp;

	if (m_emLevel > LEVEL_INFO) return;
	va_start(vp,pattern);
	logva(LEVEL_INFO,pattern,vp);
	va_end(vp);
}

void Logger::debug(const char * pattern,...)
{
	va_list vp;

	if (m_emLevel > LEVEL_DEBUG) return;
	va_start(vp,pattern);
	logva(LEVEL_DEBUG,pattern,vp);
	va_end(vp);
}

/*********************/
/* 输出16进制数据    */
/*********************/
void Logger::debug16(const char* info, const BYTE* pData, int Datasize)
{
	struct tm *now;
	time_t ltime;
	char   szName[_MAX_PATH];

	if (m_emLevel > LEVEL_DEBUG) return;
	time(&ltime);
	if (NULL == (now=localtime(&ltime))) return;

	SYSTEMTIME system;
	GetLocalTime(&system);

	m_mutex.lock();

	if (!m_file.empty())
	{
		if (m_day != now->tm_mday)
		{
			if (NULL != m_fp_file)
			{
				fclose(m_fp_file);
			}
			m_day = now->tm_mday;
			_snprintf(szName,sizeof(szName),"%s%04d%02d%02d.log",m_file.c_str(),now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
			m_fp_file = fopen(szName,"at");
		}
	}
	if (NULL != m_fp_console)
	{
		fprintf(m_fp_console,"[%s] ",m_strLogName.c_str());
	}
	if (NULL != m_fp_file)
	{
		fprintf(m_fp_file,"[%s] ",m_strLogName.c_str());
	}

	if (NULL != m_fp_console)
	{
		fprintf(m_fp_console,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(m_fp_console,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}
	if (NULL != m_fp_file)
	{
		fprintf(m_fp_file,"%04d/%02d/%02d ",now->tm_year + 1900,now->tm_mon + 1,now->tm_mday);
		fprintf(m_fp_file,"%02d:%02d:%02d.%03d ",system.wHour,system.wMinute,system.wSecond, system.wMilliseconds);
	}

	if (NULL != m_fp_console)
	{
		fprintf(m_fp_console, "%s 长度 = %u:\n",info, Datasize );
		for(int i = 0; i < Datasize; i ++)
		{
			fprintf(m_fp_console,"%2.2X ", pData[i]);
		}
		fprintf(m_fp_console,"\n\n");
		fflush(m_fp_console);
	}
	if (NULL != m_fp_file)
	{
		fprintf(m_fp_file, "%s 长度 = %u :\n",info, Datasize);
		for(int i = 0; i < Datasize;i ++)
		{
			fprintf(m_fp_file,"%2.2X ", pData[i]);
		}
		fprintf(m_fp_file,"\n\n");
		fflush(m_fp_file);
	}

	m_mutex.unlock();
}