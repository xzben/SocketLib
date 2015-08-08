#include <iostream>
#include "TimerWorker.h"
#include "WorkerPool.h"
#include "SocketDriver.h"
#include "GlobalController.h"
#include "MasterServer.h"
using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("please input a config file!!\n");
		return 1;
	}
	const char* config_file = argv[1];

	if (!CGlobalController::getInstance()->getConfig()->init(config_file))
	{
		printf("please give a valid config file!!!\n");
		return 1;
	}
	
	printf("server is starting running.....\n");

	CGlobalController::getInstance()->init();
	//先实例化时间模块对时间进行初始化
	CTimerWorker::getInstance();

	CWorkerPool::getInstance()->start();
	//CSocketDriver::getInstance()->start();
	CTimerWorker::getInstance()->start();
	
	CMasterServer::getInstance();
	CGlobalController::getInstance()->waitSystemStop();

	printf("server is stoped.....\n");
	return 0;
}
