/********************************************************************************
*	文件名称:	TimerWorker.h													*
*	创建时间：	2015/03/29														*
*	作   者 :	xzben															*
*	文件功能:	系统时间管理器，可以通过它快速获得当前时间和添加定时器				*
*********************************************************************************/
#ifndef __2015_03_29_TIMER_WORKDER_H__
#define __2015_03_29_TIMER_WORKDER_H__

#include <cstdint>
#include "common.h"
#include "Thread.h"
#include "SingleInstance.h"

class TimerCore;
class CTimerWorker : public CThread, public SingleInstance<CTimerWorker>
{
public:
	virtual bool start() override;
	//向定时器添加定时操作
	// delay_msec 单位为10 毫秒
	int32_t	addTimeout(uint32_t delay_msec, SERVER_HANDLE hanle, uint32_t session_id, CData *pData = nullptr);
	//获取当前时间戳
	uint64_t	getTime();

	//获取当前本地时间描述字符串
	std::string getFormatTime(std::string fmt = "%Y-%m-%d %H:%M:%S");
	struct tm	getCurTime();
	//获取开始运行时间戳
	uint64_t getStartTime();

	//获取当前系统的本地时间
	int32_t getTimeOfDay(struct timeval *tp, void *tzp);
	//获取当前系统的本地时间
	struct tm getSystemLocalTime();
protected:
	virtual void run() override;
	

	// 定时器内部更新，此接口应该由一个放在一个线程中不断的去更新执行
	int32_t	updateTimer();

	friend class SingleInstance<CTimerWorker>;
	CTimerWorker();
	virtual ~CTimerWorker();
private:

	TimerCore *m_pCore;
};
#endif//__2015_03_29_TIMER_WORKDER_H__