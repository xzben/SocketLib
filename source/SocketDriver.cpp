#include "SocketDriver.h"
#include "GlobalController.h"
#include "MasterServer.h"
#include <iostream>

CSocketDriver::CSocketDriver()
{

}

CSocketDriver::~CSocketDriver()
{

}

void CSocketDriver::initInstance()
{

}

void CSocketDriver::run()
{
	ThreadGuard guard;
	while (!CGlobalController::getInstance()->isSystemStop())
	{

	}
}