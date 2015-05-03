#include "GlobalController.h"
#include "WorkerPool.h"
#include "ServerManager.h"
#include "TimerWorker.h"

#include <iostream>
#include <thread>
#include <mutex>

CGlobalController::CGlobalController()
{
	m_bIsSystemStop = false;
	m_workThreadNum = 8;
	m_nRunThreadNum = 0;
	m_logLevel = LOG_LEVEL::LOG_DEBUG;
	m_logFile = "";
}

CGlobalController::~CGlobalController()
{

}

void CGlobalController::initInstance()
{
	
}

void CGlobalController::stopSystem()
{
	m_bIsSystemStop = true;
}

void CGlobalController::waitSystemStop()
{
	while (AtomGet(&m_nRunThreadNum) > 0)
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}