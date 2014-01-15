#include "IOCP.h"
#include "Acceptor.h"
#include "ClientSocket.h"

class ClientStream : public ClientSocket
{
public:
	virtual bool OnOpen()
	{
		std::string strIp; u_short nPort;
		this->getPeerAddr(strIp, nPort);

		Logger::getInstace()->debug("Connect Ip: %s --- Port: %d", strIp.c_str(), nPort);

		return true;	
	}
	
	virtual bool OnMsg()
	{
		std::string strIp; u_short nPort;
		this->getPeerAddr(strIp, nPort);

		m_printMutex.lock();
		printf("recv Ip: %s --- Port: %d\n", strIp.c_str(), nPort);
		m_printMutex.unlock();

		char  buf[1000];
		int rdSize = ReadData(buf, 1000);
		buf[rdSize] = '\0';

		m_printMutex.lock();
		printf("recv: %s\n", buf);
		m_printMutex.unlock();

		SendData(buf, rdSize);
		return true;
	}

	virtual bool OnClosed()
	{
		std::string strIp; u_short nPort;
		this->getPeerAddr(strIp, nPort);
	
		Logger::getInstace()->debug("disConnect Ip: %s --- Port: %d", strIp.c_str(), nPort);


		return true;
	}
private:
	Mutex	m_printMutex;
};

int main(int argc, char *argv[])
{
	Logger::getInstace()->setLevel(Logger::LEVEL_DEBUG);
	Logger::getInstace()->addLocalFileLog("SocketLib");

	Acceptor<ClientStream> acceptor;
	acceptor.startServer(6000);
	
	printf("server started......\n");
	
	IOCPDriver::getInstance()->wait_event_loop(); //等待工作线程结束

	return 0;
}