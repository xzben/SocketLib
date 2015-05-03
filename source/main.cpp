#include <iostream>
#include "config.h"
#include "TimerWorker.h"
#include "WorkerPool.h"
#include "ServerManager.h"
#include "SocketDriver.h"
#include "GlobalController.h"
#include "LogServer.h"
#include "CoroutineScheduler.h"
#include "MasterServer.h"
#include "ConfigFile.h"
#include <windows.h>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("请传递一个初始化的配置文件\n");
		return 1;
	}
	const char* config_file = argv[1];

	if (!CConfigFile::getInstance()->init(config_file))
	{
		printf("请传递一个有效的初始化的配置文件\n");
		return 1;
	}

	//全局控制器必须第一个启动初始化
	CGlobalController::getInstance();

	//实例化一个日志服务
	CLogServer::getInstance();
	CMasterServer::getInstance();
	//先实例化时间模块对时间进行初始化
	CTimerWorker::getInstance();

	CWorkerPool::getInstance()->start();
	CSocketDriver::getInstance()->start();
	CTimerWorker::getInstance()->start();
	
	CGlobalController::getInstance()->waitSystemStop();
	
	return 0;
}