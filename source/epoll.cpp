#include "epoll.h"
#include "GlobalController.h"
#include "SocketDriver.h"
#include "socket.h"
#include <unordered_map>

#if (CUR_PLATFROM == PLATFROM_LINUX)

struct EpollData
{
public:
	static EpollData* create()
	{
		return new EpollData;
	}

	EpollData()
	{
		sock = INVALID_SOCKET;
		type = IO_UNKNOW;
		package = nullptr;
		sendPackage = nullptr;
	}

	SOCKET_HANDLE sock;
	SocketIOType  type;
	UINT		  events;
	Package*	  package; //recv
	Package*	  sendPackage; //send
};

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
};

EpollDriver::EpollDriver()
{
	m_epollHandle = epoll_create(1024);
	m_pEvtDataManager = new EpollEventDataManager;
}

EpollDriver::~EpollDriver()
{
	close(efd);
}

int32_t EpollDriver::poll_event_process(IOEvent *ioEvents, int32_t max, int waittime /*=-1*/)
{
	struct epoll_event events[max];
	LOG_INFO("May the source be with you!!");

	int fds = epoll_wait(m_epollHandle, events, max, waittime);
	if (fds == 0)
	{
		LOG_INFO("event loop timed out");
		return 0;
	}

	for (int i = 0;; i < fds; i++)
	{
		EpollData* data = (EpollData*)events[i].data.ptr;
		IOEvent ioEvent;
		SOCKET_HANDLE evtSock = data->sock;
		ioEvent.evt_sock = data->sock;
		
		if (data)
		{
			LOG_INFO("started processing for event id(%d) and sock(%d)", i, evtSock);
			// when data avaliable for read or urgent flag is set
			if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
			{
				if (events[i].events & EPOLLIN)
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
				if(data->type & IO_Accept)
				{
					Socket ListenSocket;
					ListenSocket.attach(evtSock);
					Socket acpSocket;
					InterAddress addrRemote;
					ListenSocket.accept(acpSocket, &addrRemote);
					ioEvent.acpt_sock = acpSocket.dettach();
					ioEvent.evt_type = IO_Accept;
				}

				if(data->type & IO_Read)
				{
					Socket recvSocket;
					recvSocket.attach(evtSock);
					Package *package = data->package;
					void *pDataBuffer = nullptr;
					PackageSize nDataSize = 0;
					package->getFillData(nDataSize, pDataBuffer);
					PackageSize readSize = recvSocket.recv(pDataBuffer, nDataSize);
					if (readSize > 0)
					{
						package->FillData(readSize);
						ioEvent.evt_type = IO_Read;

						if (package->isFillComplete())
						{
							CSocketDriver::getInstance()->resetSocketPackage(evtSock);
						}
						poll_recv(evtSock);
						ioEvent.package = package;
					}
					else
					{
						LOG_ERROR("EpollDriver recv data failed event id(%d) and sock(%d)", i, evtSock);
						ioEvent.evt_type = IO_Close;
					}
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				LOG_INFO("found EPOLLOUT for event id(%d) and sock(%d)", i, evtSock);

				if (data->type & IO_Write)
				{
					Package *package = data->sendPackage;
					void *pDataBuffer = nullptr;
					PackageSize nDataSize = 0;
					package->getSendData(nDataSize, pDataBuffer);

					Socket sendSocket;
					sendSocket.attach(evtSock);
					PackageSize sendSize = sendSocket.send(pDataBuffer, nDataSize);
					if (sendSize > 0)
					{
						package->offsetData(sendSize);
						ioEvent.evt_type = IO_Write;
						if (package->isSendComplete())
						{
							Package::free(package);
							data->sendPackage = nullptr;
							data->events &= 
							struct epoll_event ev;
							ev.events &= ~EPOLLOUT;
							ev.data.ptr = data;
							epoll_ctl(m_epollHandle, EPOLL_CTL_MOD, evtSock, &ev);
						}
					}
					else
					{
						LOG_ERROR("EpollDriver send data failed event id(%d) and sock(%d)", i, evtSock);
						ioEvent.evt_type = IO_Close;
						Package::free(package);
					}
				}
			}
			// shutdown or error
			else if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
			{
				if (events[i].events & EPOLLRDHUP)
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
			ioEvent.evt_type = IO_Close;
		}
		ioEvents[i] = ioEvent;
	}

	return fds;
}

int32_t EpollDriver::poll_add(SOCKET_HANDLE sock)
{
	EpollData* data = m_pEvtDataManager->getEpollData(sock);
	data->sock = sock;
	data->package = nullptr;
	data->events = EPOLLIN;
	data->type = IO_UNKNOW;

	struct epoll_event ev;
	ev.events = data->events;
	ev.data.ptr = data;

	if (epoll_ctl(m_epollHandle, EPOLL_CTL_ADD, sock, &ev) == -1) {
		return 1;
	}
	return 0;
}

int32_t EpollDriver::poll_del(SOCKET_HANDLE sock)
{
	m_pEvtDataManager->freeEpollData(sock);
	epoll_ctl(m_epollHandle, EPOLL_CTL_DEL, sock , NULL);
	return 0;
}

int32_t EpollDriver::poll_listen(SOCKET_HANDLE sock)
{
	EpollData* data = m_pEvtDataManager->getEpollData(sock);
	if (poll_add(sock) != 0)
		return -1;

	data->type |= IO_Accept;

	return 0;
}

int32_t EpollDriver::poll_connect(const short port, const char* ip)
{
	return 0;
}

int32_t EpollDriver::poll_send(SOCKET_HANDLE sock, void* buf, int32_t sz)
{
	Package *package = Package::create(sz, buf);
	if (package->isFillComplete())
	{
		EpollData* data = m_pEvtDataManager->getEpollData(sock);
		data->sock = sock;
		data->type |= IO_Write;
		data->events |= EPOLLOUT;
		data->sendPackage = package;

		struct epoll_event ev;
		ev.events = data->events;
		ev.data.ptr = data;
		epoll_ctl(m_epollHandle, EPOLL_CTL_MOD, sock, &ev);
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
	EpollData* data = m_pEvtDataManager->getEpollData(sock);
	
	data->events |= EPOLLIN;
	data->type |= IO_Read;
	data->sock = sock;
	data->package  = CSocketDriver::getInstance()->getSocketPackage(evtSock);

	return 0;
}

#endif//(CUR_PLATFROM == PLATFROM_LINUX)