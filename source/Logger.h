#ifndef __2014_03_02_CLOGGER_H__
#define __2014_03_02_CLOGGER_H__

#include "CObject.h"
#include "Mutex.h"

/*
*	系统日志，按天为单位存储日志文件
*	可以设置日志显示等级, 具体等级信息看类中的 LOG_LEVEL 枚举
*/
class Logger : public NoCopyable
{
public:
	/*
	*	日志等级, 通过日志等级可以设置日志中显示的信息
	*/
	typedef enum
	{
		/*
		*	关闭日志功能，将不会显示任何信息
		*/
		LEVEL_OFF = INT_MAX,
		/*
		*	设置为LEVEL_FATAL时，显示等于此等级的信息
		*	致命错误: 此类错误是将导致系统崩溃的
		*/
		LEVEL_FATAL = 500000,
		/*
		*	设置为LEVEL_ERROR时，显示等级大于等于此等级的信息
		*	错误:	此类为系统中一些错误，但是系统还能正常运行
		*/
		LEVEL_ERROR = 400000,		   //	错误等级，显示
		/*
		*	设置为LEVEL_WARN时，显示等级大于等于此等级的信息
		*	警告: 系统中一些需要注意的地方, 但是还不属于错误，但是可能是后面错误产生的根源
		*/
		LEVEL_WARN  = 300000,
		/*
		*	设置为LEVEL_INFO时，显示等级大于等于此等级的信息
		*	信息: 系统中一些的一些运行信息，用于跟踪服务器的状态
		*/
		LEVEL_INFO  = 20000,
		/*
		*	设置为LEVEL_DEBUG时，显示全部的信息
		*/
		LEVEL_DEBUG = 10000,
		/*
		*	设置为LEVEL_ALL时，显示全部的信息
		*/
		LEVEL_ALL = INT_MIN, 
	}LOG_LEVEL;
	
	~Logger(); 
	static Logger* getInstace()
	{
		static Logger instance;
		return &instance;
	}
	void removeConsoleLog();
	void addLocalFileLog(const std::string &file);

	void setLevel(const LOG_LEVEL level);
	void setLevel(const std::string &level);

	void logtext(const LOG_LEVEL level,const char * text);
	void logva(const LOG_LEVEL level,const char * pattern,va_list vp);
	void log(const LOG_LEVEL level,const char * pattern,...);

	void debug(const char * pattern,...);
	void debug16(const char* info, const BYTE* pData, int Datasize);
	void error(const char * pattern,...);
	void info(const char * pattern,...);
	void fatal(const char * pattern,...);
	void warn(const char * pattern,...);
	void setLoggerName(std::string strLogNmae){ m_strLogName = strLogNmae; }
protected:
	Logger(std::string strLogName = "LOGGER", LOG_LEVEL emLevel= LEVEL_ERROR);
private:
	Mutex			m_mutex;
	LOG_LEVEL		m_emLevel;
	std::string		m_strLogName;
	std::string		m_file;
	FILE			*m_fp_console;
	FILE			*m_fp_file;		
	int				m_day;			//当前打开日志的是哪天的，每天一个日志文本
};
#endif//__2014_03_02_CLOGGER_H__