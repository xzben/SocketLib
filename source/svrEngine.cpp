#include "svrEngine.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
//Thread
DWORD __stdcall Thread::ThreadFunc(void *arg)
{
	static volatile int s_RunningThreadNumberCounter = 0;
	s_RunningThreadNumberCounter ++;

	Thread *pThead = (Thread*)arg;
	//调试信息
	Logger::getInstace()->debug("Thread %s is start.\r\nCurrent Thread number[%d]", pThead->m_strThreadName.c_str(), s_RunningThreadNumberCounter);
	//线程开始运行设置运行标志
	pThead->m_ThreadMutex.lock();
	pThead->m_bActive = true;
	pThead->m_bComplete = false;
	pThead->m_ThreadMutex.unlock(); 
	
	//线程主回调
	pThead->run();
	
	//线程结束设置运行标志
	pThead->m_ThreadMutex.lock();
	pThead->m_bActive = false;
	pThead->m_bComplete = true;
	pThead->m_ThreadMutex.unlock();

	s_RunningThreadNumberCounter --;
	Logger::getInstace()->debug("Thread[%s] is End.\r\nCurrent Thread Number[%d]", pThead->m_strThreadName.c_str(), s_RunningThreadNumberCounter);
	
	//如果 对象时不可 Join 的则在线程结束时自动将对象删除
	if( !pThead->isJoinable() )
	{
		SAFE_DELETE(pThead);
	}
	else //如果对象为可 Join 的则 线程结束时只是关闭线程句柄，线程对象由 join 的线程去删除
	{
		::CloseHandle(pThead->m_hThreadHandle);
		pThead->m_hThreadHandle = NULL;
	}

	return 0;
}

void Thread::join()
{
	WaitForSingleObject(m_hThreadHandle, INFINITE);
}

bool Thread::start()
{
	if(m_bActive) //如果已经运行了，直接返回
		return false;

	DWORD dwThreadID;
	m_hThreadHandle = CreateThread(NULL,
		0, 
		(LPTHREAD_START_ROUTINE)ThreadFunc,
		(void*) this, 
		0, 
		&dwThreadID);

	if(NULL == m_hThreadHandle)
	{
		return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
//TCPSocket
sockaddr_in TCPSocket::s_getSockAddrIpV4(const char* szIpAddr, const u_short usPort)
{
	sockaddr_in	addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(usPort);
	if(NULL == szIpAddr)
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	else
		addr.sin_addr.S_un.S_addr = inet_addr(szIpAddr);

	return addr;
}
void	TCPSocket::s_getSockAddrIpV4(const sockaddr_in addr, std::string& strIpAddr, u_short &usPort)
{
	usPort = ntohs(addr.sin_port);
	strIpAddr = inet_ntoa(addr.sin_addr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
TCPSocket::TCPSocket()
{
	m_hSocket = INVALID_SOCKET;
	m_bHaveAddr = false;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));
}

bool TCPSocket::init(const int nAf /*= AF_INET*/, const int nProtocl /*= 0*/)
{
	m_hSocket = ::socket(nAf, SOCK_STREAM, nProtocl);
	if(INVALID_SOCKET == m_hSocket)
		return false;
	m_bHaveAddr = false;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));

	return true;
}

TCPSocket::~TCPSocket()
{
	this->close();
}

bool TCPSocket::attach(SOCKET hSocket)
{
	if(INVALID_SOCKET == hSocket)
		return false;

	m_hSocket = hSocket;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));
	m_bHaveAddr = false;
	return true;
}

SOCKET TCPSocket::dettach()
{
	SOCKET skRet = m_hSocket;
	m_hSocket = INVALID_SOCKET;
	memset(&m_addrPeer, 0, sizeof(m_addrPeer));
	m_bHaveAddr = false;
	return skRet;
}

bool TCPSocket::bind(const sockaddr_in& addrBind)
{
	if( SOCKET_ERROR == ::bind(m_hSocket, (sockaddr*)&addrBind, sizeof(sockaddr)))
	{
		int nErr = ::GetLastError();
		//常见的err 为 WSAEADDRINUSE  10048
		Logger::getInstace()->error("TCPSocket >> Bind address faild error id: %d", nErr);
		return false;
	}
	return true;
}

bool TCPSocket::listen(int nBacklog /*= 10*/)
{
	if(SOCKET_ERROR == ::listen(m_hSocket, nBacklog))
	{
		int nErr = ::GetLastError();
		//常见的err 为 WSAEADDRINUSE  10048
		Logger::getInstace()->error("TCPSocket >> listen faild error id: %d", nErr);
		return false;
	}
	return true;
}

bool TCPSocket::startServer(int nPort)
{
	sockaddr_in addr = s_getSockAddrIpV4(NULL, nPort);
	if(!this->bind(addr))  return false;

	return this->listen();
}

TCPSocket*	TCPSocket::accept()
{
	TCPSocket *pNewSocket = new TCPSocket;
	sockaddr_in	conAddr;
	memset(&conAddr, 0, sizeof(conAddr));
	int nAddrLen = sizeof(conAddr);

	SOCKET hConnSocket = ::accept(m_hSocket, (sockaddr*)&conAddr, &nAddrLen);
	if(INVALID_SOCKET == hConnSocket)
	{
		Logger::getInstace()->error("TCPSocket >> accept 返回一个INVALID_SOCKET");
		delete pNewSocket;
		return NULL;
	}
	pNewSocket->attach(hConnSocket);
	pNewSocket->setPeerAddr(conAddr);
	return pNewSocket;
}

bool	TCPSocket::connect(const sockaddr_in& addrCon)
{
	if( SOCKET_ERROR == ::connect(m_hSocket, (sockaddr*)&addrCon, sizeof(addrCon)) )
	{
		int nErr = ::GetLastError();
		Logger::getInstace()->error("TCPSocket >> Connect failed error id: %d", nErr);
		return false;
	}
	setPeerAddr(addrCon);
	return true;
}

int	TCPSocket::recv(char* pBuf, int nLen, int nFlag /* =0*/)
{
	int nSize = ::recv(m_hSocket, pBuf, nLen, nFlag);
	if(nSize == 0)
	{
		return -2;
	}
	else if(nSize == SOCKET_ERROR)
	{
		return -1;
	}
	return nSize;
}

int	TCPSocket::send(const char *pBuf,const int nLen, int nFlag/* =0*/)
{
	int nSize;
	if( SOCKET_ERROR == (nSize = ::send(m_hSocket, pBuf, nLen, nFlag)) )
	{
		int nErr = ::GetLastError();
		return -1;
	}
	return nSize;
}

bool TCPSocket::close()
{
	if(INVALID_SOCKET == m_hSocket)
		return true;

	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;
	return true;
}

bool TCPSocket::disConnect()
{
	this->shutdown();
	return this->close();
}

bool TCPSocket::getLocalAddr(std::string& strIpAddr, u_short& usPort)
{
	sockaddr_in addr;
	int nLen = sizeof(addr);
	if(0 == getsockname(m_hSocket, (sockaddr*)&addr, &nLen))
	{
		s_getSockAddrIpV4(addr, strIpAddr, usPort);
		return true;
	}
	return false;
}

bool TCPSocket::getPeerAddr(std::string& strIpAddr, u_short& usPort)
{
	if(!m_bHaveAddr && !initPeerAddress() )
		return false;

	s_getSockAddrIpV4(m_addrPeer, strIpAddr, usPort);
	return true;
}

bool TCPSocket::setPeerAddr(const sockaddr_in& addrPeer)
{
	m_addrPeer = addrPeer;
	m_bHaveAddr = true;
	return true;
}

SOCKET	TCPSocket::getSocket()
{
	return m_hSocket;
}

bool TCPSocket::updateAcceptContext()
{
	return 0 == setsockopt( m_hSocket,
		SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char *)&(m_hSocket), sizeof(m_hSocket) );
}

bool TCPSocket::initPeerAddress()
{
	assert( INVALID_SOCKET != m_hSocket );
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);

	if ( SOCKET_ERROR == getpeername( m_hSocket,
		(sockaddr*)&sockAddr, &nSockAddrLen ) ) return false;

	m_addrPeer = sockAddr;
	m_bHaveAddr = true;
	return true;
}

bool TCPSocket::shutdown(int how /*= SD_BOTH*/)
{
	if(m_hSocket == INVALID_SOCKET)	return true;
	return 0 == ::shutdown(m_hSocket, how);
}

//////////////////////////////////////////////////////////////////////////
//用于自动装载 SocketLib 用
class SocketLibLoadHelper
{
public:
	SocketLibLoadHelper()
	{
		s_loadSockLib();
	}
	~SocketLibLoadHelper()
	{
		s_destroySockLib();
	}
private:
	static bool			s_loadSockLib(int nHigh = 2, int nLow = 2);
	static bool			s_destroySockLib();
private:
	static	bool		s_bLoadedSockLib;
};
bool SocketLibLoadHelper::s_bLoadedSockLib = false;
bool SocketLibLoadHelper::s_loadSockLib(int nHigh /* = 2 */, int nLow /* = 2 */)
{
	if( s_bLoadedSockLib ) //已经加载过，直接返回
		return true;
	s_bLoadedSockLib = true;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( nHigh, nLow );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
		return false;

	if ( LOBYTE( wsaData.wVersion ) != nHigh 
		|| HIBYTE( wsaData.wVersion ) != nLow )
	{
		WSACleanup( );
		return false;
	}
	return true;
}
bool SocketLibLoadHelper::s_destroySockLib()
{
	if( !s_bLoadedSockLib ) //还未加载过，或已经卸载了
		return true; 

	s_bLoadedSockLib = false;
	if(0 != WSACleanup())
		return false;

	return true;
}

SocketLibLoadHelper g_socketLibLoadHelper; //定义一个全局变量，使运行环境自动装载和卸载 SocketLib
//////////////////////////////////////////////////////////////////////////

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