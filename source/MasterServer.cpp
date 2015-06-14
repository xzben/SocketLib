#include "MasterServer.h"
#include <cassert>
#include "TimerWorker.h"
#include "GlobalController.h"

CMasterServer::CMasterServer()
{
	m_count = 0;
}

CMasterServer::~CMasterServer()
{

}


void CMasterServer::initInstance()
{
	timeout(100, SERVER_TIMEOUT_SELECTOR(CMasterServer::tick));
}

void CALLBACK CMasterServer::tick(CData *pData)
{
	if (m_count < 10)
	{
		LOG_INFO("test... timeout %s", CTimerWorker::getInstance()->getFormatTime().c_str());
		timeout(100, SERVER_TIMEOUT_SELECTOR(CMasterServer::tick));
	}
	else
		CGlobalController::getInstance()->stopSystem();
	m_count++;

}

void CMasterServer::excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)
{

}