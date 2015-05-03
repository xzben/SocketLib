/********************************************************************************
*	文件名称:	GlobalController.h												*
*	创建时间：	2015/04/09														*
*	作   者 :	xzben															*
*	文件功能:	系统的全局控制器，用于控制服务器的公共操作							*
*********************************************************************************/

#ifndef __2015_04_09_GLOBAL_CONTROLLER_H__
#define __2015_04_09_GLOBAL_CONTROLLER_H__

#include "SingleInstance.h"
#include "Condition.h"
#include "common.h"
#include "Atom.h"
#include <map>

class CGlobalController : public SingleInstance<CGlobalController>
{
public:
	uint32_t		getWorkThreadNum(){ return m_workThreadNum; }
	bool			isSystemStop(){ return m_bIsSystemStop; }
	LOG_LEVEL		getLogLevel() { return m_logLevel; }
	std::string&	getLogFileName() { return m_logFile; }
	void			stopSystem();
	void			waitSystemStop();
	
protected:
	friend class SingleInstance<CGlobalController>;
	virtual void initInstance() override;
	friend class ThreadGuard;
	void			addRunThread(){ AtomSelfAdd(&m_nRunThreadNum); }
	void			decRunThread(){ AtomSelfDec(&m_nRunThreadNum); }
	CGlobalController();
	virtual ~CGlobalController();
private:
	uint32_t	m_nRunThreadNum;
	uint32_t	m_workThreadNum;
	bool		m_bIsSystemStop;
	LOG_LEVEL	m_logLevel;
	std::string m_logFile;
	std::map<std::string, std::string> configs;
};

class ThreadGuard
{
public:
	ThreadGuard()
	{
		CGlobalController::getInstance()->addRunThread();
	}
	~ThreadGuard()
	{
		CGlobalController::getInstance()->decRunThread();
	}
};
#endif//__2015_04_09_GLOBAL_CONTROLLER_H__