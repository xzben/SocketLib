#include "epoll.h"
#include "GlobalController.h"
#include "SocketDriver.h"
#include "socket.h"
#include "RWLock.h"
#include <unordered_map>
#include <queue>

#if (CUR_PLATFROM == PLATFORM_UNKNOW)
///////////////////////////////////////////////////////

struct EpollData
{
public:
	static EpollData* create()
	{
		return new EpollData;
	}

	static void free(EpollData* obj)
	{
		delete obj;
	}

	EpollData()
	{
		sock = INVALID_SOCKET;
		type = IO_UNKNOW;
		recvPackage = nullptr;
	}

	~EpollData()
	{
		Package::free(recvPackage);
		while (!sendPackages.empty())
		{
			Package::free(sendPackages.pop());
		}
	}
	SOCKET_HANDLE sock;
	SocketIOType  type; //SocketIOType
	uint32_t	  events;
	Package*	  recvPackage; //recv
	std::queue<Package*> sendPackages;
	RWLcok		  lock;
};
////////////////////////////////////////////////////
class EpollEventDataManager
{
public:
	EpollEventDataManager();
	~EpollEventDataManager();

	EpollData* getEpollData(SOCKET_HANDLE sock);
	void	   freeEpollData(SOCKET_HANDLE sock);

private:
	typedef std::unordered_map<SOCKET_HANDLE, EpollData*>  Sock2EpollData;
	Sock2EpollData	m_sockDatas;
	std::mutex      m_lock;

};

EpollEventDataManager::EpollEventDataManager()
{

}

EpollEventDataManager::~EpollEventDataManager()
{
	for (auto it = m_sockDatas.begin(); it != m_sockDatas.end(); it++)
	{
		EpollData::free(it->second);
	}
	m_sockDatas.clear();
}

EpollData* EpollEventDataManager::getEpollData(SOCKET_HANDLE sock)
{
	EpollData* ret = nullptr;

	m_lock.lock();
	auto it = m_sockDatas.find(sock);
	if (it == m_sockDatas.end())
	{
		ret = EpollData::create();
	}
	else
	{
		ret = it->second;
	}
	m_lock.unlock();

	return ret;
}

void EpollEventDataManager::freeEpollData(SOCKET_HANDLE sock)
{
	m_lock.lock();
	auto it = m_sockDatas.find(sock);
	if (it != m_sockDatas.end())
	{
		EpollData::free(it*);
		m_sockDatas.erase(it);
	}
	m_lock.unlock();
}

//////////////////////////////////////////////////////////////////////
EpollDriver::EpollDriver()
{
	m_epollHandle = epoll_create(1024);
	m_pEvtDataManager = new EpollEventDataManager;
}

EpollDriver::~EpollDriver()
{
	close(m_epollHandle);
	SAFE_DELETE(m_pEvtDataManager);
}

int32_t EpollDriver::poll_event_process(IOEvent *events, int32_t max, int waittime /*=-1*/)
{
	struct epoll_event poll_events[max];

	int fds = epoll_wait(m_epollHandle, poll_events, max, waittime);
	if (fds == 0)
	{
		LOG_INFO("%s","event loop timed out");
		return 0;
	}

	int indx = 0;
	for (int i = 0; i < fds; i++)
	{
		EpollData* data = (EpollData*)poll_events[i].data.ptr;

		IOEvent ioEvent;
		data->lock.rLock();
		SOCKET_HANDLE evtSock = data->sock;
		SocketIOType  curType = data->type;
		data->lock.rUnlock();
		uint32_t	  curEvent = poll_events[i].events;
		ioEvent.evt_sock = evtSock;

		if (data)
		{
			LOG_INFO("started processing for event id(%d) and sock(%d)", i, evtSock);
			// when data avaliable for read or urgent flag is set
			if ((curEvent & EPOLLIN) || (curEvent & EPOLLPRI))
			{
				if (curEvent & EPOLLIN)
				{
					LOG_INFO("found EPOLLIN for event id(%d) and sock(%d)", i, evtSock);
					//value->cur_event &= EPOLLIN;
				}
				else
				{
					LOG_INFO("found EPOLLPRI for event id(%d) and sock(%d)", i, evtSock);
					//value->cur_event &= EPOLLPRI;
				}
				/// connect or accept callbacks also go through EPOLLIN
				/// accept callback if flag set
				if(curType & IO_Accept)
				{
					Socket ListenSocket;
					ListenSocket.attach(evtSock);
					Socket acpSocket;
					InterAddress addrRemote;
					ListenSocket.accept(acpSocket, &addrRemote);
					ioEvent.acpt_sock = acpSocket.dettach();
					ioEvent.evt_type = IO_Accept;
				}

				if(curType & IO_Read)
				{
					Socket recvSocket;
					recvSocket.attach(evtSock);

					data->lock.rLock();
					Package *package = data->package;
					data->lock.rUnlock();

					if (package == nullptr)
						package = CSocketDriver::getInstance()->getSocketPackage(evtSock);

					PackageSize readSize = 0;
					if (package != nullptr)
					{
						void *pDataBuffer = nullptr;
						PackageSize nDataSize = 0;
						package->getFillData(nDataSize, pDataBuffer);
						readSize = recvSocket.recv(pDataBuffer, nDataSize);
					}
					
					if (readSize > 0)
					{
						package->FillData(readSize);
						ioEvent.evt_type = IO_ReadPart;

						if (package->isFillComplete())
						{
							ioEvent.evt_type = IO_Read;
							ioEvent.package = package;
							package = CSocketDriver::getInstance()->resetSocketPackage(evtSock);
						}
						recv(evtSock, package);	
					}
				}
			}
			else if (curEvent & EPOLLOUT)
			{
				LOG_INFO("found EPOLLOUT for event id(%d) and sock(%d)", i, evtSock);
				if (curType & IO_Write)
				{
					Package *package = nullptr;
					
					data->lock.rLock();
					if (!data->sendPackages.empty())
						package = data->sendPackages.front();
					data->lock.rUnlock();
					
					PackageSize sendSize = 0;
					if (package != nullptr)
					{	
						void *pDataBuffer = nullptr;
						PackageSize nDataSize = 0;
						package->getSendData(nDataSize, pDataBuffer);

						Socket sendSocket;
						sendSocket.attach(evtSock);
						sendSize = sendSocket.send(pDataBuffer, nDataSize);
					}

					if (sendSize > 0)
					{
						package->offsetData(sendSize);
						ioEvent.evt_type = IO_Write;
						if (package->isSendComplete())
						{
							data->lock.wLock();
							data->sendPackages.pop();
							bool isEmpty = data->sendPackages.empty();
							data->lock.wUnlock;

							if (isEmpty)
							{
								clearEvent(event, EPOLLOUT, IO_Write);
							}

							Package::free(package);
						}
					}
				}
			}
			// shutdown or error
			else if ((curEvent & EPOLLRDHUP) || (curEvent & EPOLLERR) || (curEvent & EPOLLHUP))
			{
				if (curEvent & EPOLLRDHUP)
				{
					LOG_INFO("found EPOLLRDHUP for event id(%d) and sock(%d)", i, evtSock);
					//value->cur_event &= EPOLLRDHUP;
				}
				else
				{
					LOG_INFO("found EPOLLERR for event id(%d) and sock(%d)", i, evtSock);
					//value->cur_event &= EPOLLERR;
				}
				ioEvent.evt_type = IO_Close;
			}
		}
		else // not in table
		{
			ioEvent.evt_type = IO_Error;
		}
		events[indx] = ioEvent;
		indx++;
	}

	return fds;
}

int32_t EpollDriver::poll_add(SOCKET_HANDLE sock)
{
	EpollData* data = m_pEvtDataManager->getEpollData(sock);

	data->lock.wLock();
	data->sock = sock;
	data->package = nullptr;
	data->events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
	data->type = IO_UNKNOW;
	

	struct epoll_event ev;
	ev.events = data->events;
	ev.data.ptr = data;

	int32_t ret = 0;
	if (epoll_ctl(m_epollHandle, EPOLL_CTL_ADD, sock, &ev) == -1)
	{
		LOG_WARN("poll_add failed cur events[%d] cur type[%d]", data->events, data->type);
		ret = -1;
	}
	data->lock.wUnlock();

	return ret;
}

int32_t EpollDriver::poll_del(SOCKET_HANDLE sock)
{
	m_pEvtDataManager->freeEpollData(sock);
	epoll_ctl(m_epollHandle, EPOLL_CTL_DEL, sock , NULL);
	return 0;
}

int32_t EpollDriver::poll_listen(SOCKET_HANDLE sock)
{
	if (poll_add(sock) != 0 || addEvent(sock, IO_Accept, EPOLLIN) != 0)
	{
		return -1;
	}
	return 0;
}

int32_t EpollDriver::poll_connect(SERVER_HANDLE handle, const short port, const char* ip)
{
	return 0;
}

int32_t EpollDriver::poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz)
{
	Package *package = Package::create(sz, buf);
	if (package->isFillComplete())
	{
		EpollData* data = m_pEvtDataManager->getEpollData(sock);

		data->lock.wLock();

		data->sock = sock;
		data->type |= IO_Write;
		data->events |= EPOLLOUT;
		data->sendPackages.push(package);

		struct epoll_event ev;
		ev.events = data->events;
		ev.data.ptr = data;
		
		uint32_t ret = 0;
		if (epoll_ctl(m_epollHandle, EPOLL_CTL_MOD, sock, &ev) == -1)
		{
			LOG_WARN("poll_send failed cur events[%d] cur type[%d]", data->events, data->type);
			ret = -1;
		}
		data->lock.wUnlock();
		return ret;
	}
	else
	{
		Package::free(package);
		return -1;
	}
	return 0;
}

int32_t EpollDriver::poll_recv(SOCKET_HANDLE sock)
{
	return recv(sock);
}
//////////////////////////////////////////////////////////////////////////
int32_t	EpollDriver::recv(SOCKET_HANDLE sock, Package* package /*= nullptr*/)
{
	package = package == nullptr ? CSocketDriver::getInstance()->getSocketPackage(sock) : package;
	if (nullptr == package)
		return -1;

	return addEvent(sock, IO_Read, EPOLLIN, package);
}

int32_t EpollDriver::clearEvent(SOCKET_HANDLE sock, uint32_t event, SocketIOType ioType)
{
	EpollData* data = m_pEvtDataManager->getEpollData(sock);
	ASSERT(data != nullptr);

	data->lock.wLock();
	DEL_BIT(data->events, event);
	DEL_BIT(data->type, ioType);

	struct epoll_event ev;
	ev.events = data->events;
	ev.data.ptr = data;

	uint32_t ret = 0;
	if (epoll_ctl(m_epollHandle, EPOLL_CTL_MOD, sock, &ev) == -1)
	{
		LOG_WARN("clear Event failed cur events[%d] cur type[%d]", data->events, data->type);
		ret = -1;
	}
	data->lock.wUnlock();
	return ret;
}

int32_t EpollDriver::addEvent(SOCKET_HANDLE sock, SocketIOType ioType, uint32_t events, Package* recvPackage /*=nullptr*/)
{
	EpollData* data = m_pEvtDataManager->getEpollData(sock);
	ASSERT(data != nullptr);

	data->lock.wLock();

	data->sock = sock;
	data->recvPackage = package;
	data->events |= events;
	data->type = ioType;

	struct epoll_event ev;
	ev.events = data->events;
	ev.data.ptr = data;

	int32_t ret = 0;
	if (epoll_ctl(m_epollHandle, EPOLL_CTL_MOD, sock, &ev) == -1)
	{
		LOG_WARN("add Event failed cur events[%d] cur type[%d]", data->events, data->type);
		ret = -1;
	}
	
	data->lock.wUnlock();
	return ret;
}
#endif//(CUR_PLATFROM == PLATFORM_UNKNOW)
