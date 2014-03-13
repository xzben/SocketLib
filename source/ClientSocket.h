#ifndef __2013_01_04_CLIENT_SOCKET_H__
#define __2013_01_04_CLIENT_SOCKET_H__

#include "CObject.h"
#include "Socket.h"
#include "IOCP.h"
#include "MemoryPool.h"
#include "IOBuffer.h"

class Event_handler
{
public:
	/*
		连接打开后调用
		用于验证连接是否成立
		如果成立返回 true，代表连接生效，会立即投递一个recv操作
		如果return false 则连接会立即关闭，不会执行 IO操作了。
		@@ 注意此接口是占用 套接字IOCP的线程处理的，所以不应该有复杂的长时间操作
	*/
	virtual bool	OnOpen() = 0;
	/*
	*	有收到信的消息数据
	*	目前返回值没有用
	*	@ 此操作由专门的消息线程处理，不会影响IOCP处理线程。
	*/
	virtual	bool	OnMsg()	 = 0;
	/*
	*	当套接字关闭时调用
	*	目前返回值没有用
	*	@@ 注意此接口是占用 套接字IOCP的线程处理的，所以不应该有复杂的长时间操作
	*/
	virtual	bool	OnClosed() = 0;
protected:
	Event_handler(){};
};

class ClientSocket : public TCPSocket, public AllocFromMemoryPool, public Event_handler
{
	template<class _ClientType>
	friend class Acceptor;

	friend class IocpRecvThread;
	friend class IOTask;
	friend class AcceptSocket;
	friend class SocketPool;
	typedef struct st_RWOverlappedData : public st_OverlappedData
	{
		st_RWOverlappedData(IOOperationType	type)
			:st_OverlappedData(type)
		{
			_wsaBuffer.buf = NULL;
			_wsaBuffer.len = 0;
		}
		WSABUF _wsaBuffer;	//WSARecv接收缓冲数据,传递给WSARecv()的第2个参数
	}RWOverlappedData, *LPRWOverlappedData;
public:
	ClientSocket();
	virtual ~ClientSocket();

	bool SendData(void *pBuffer, const int nSize);

	int	ReadData(void *pBuffer,	const int nSize, bool bDel = true);

	//判断链接是否已经失效
	bool isFinal()
	{
		AutoLock lock(&m_mutex);
		return m_bComplete;
	}
	
	//获得连接最近的心跳
	time_t	getLastHeart()
	{
		AutoLock lock(&m_mutex);
		return m_tmLastHeart;
	}
	
protected:
	//刷新心跳
	void	refreshHeart()
	{
		AutoLock lock(&m_mutex);
		m_tmLastHeart = time(NULL);
	}

	bool	addSend();
	bool	addRecv();
	//增加连接的使用计数
	void addReference()
	{
		AutoLock lock(&m_mutex);
		m_reference ++;
	}
	//减小链接的使用计数，当小于等于0时删除链接
	// 返回 true 说明 减引用后对象还是有效的，否则说明对象已经被 delete 了
	bool	decReference()
	{
		AutoLock lock(&m_mutex);
		m_reference --;
		if(m_reference <= 0)
		{
			delete this;
			return false;
		}
		return true;
	}
	//终止链接使其失效, 并调用 OnClosed handler 告诉用户连接关闭
	void final()
	{
		AutoLock lock(&m_mutex);
		if( !m_bComplete)
		{
			this->disConnect();
			this->OnClosed();
		}
		m_bComplete = true;

	}
	// 将 ClientSocket 连接对象，连接终止，并减引用，尝试删除对象
	// 返回 true ，虽然 ClientSocket 对象连接关闭了，但是由于还有被引用，所以当前对象还是有效的
	// 返回 false 说明 对象内存已经成功被delete了，所以对象不能再被使用了。
	bool	release()
	{
		final();
		return decReference();
	}
	bool	sendout(int nSendDoneSize);
	bool	recvDone(int nRecvDoneSize);
private:
	static	volatile unsigned int s_ClientSocketCount;
	unsigned int		m_nID;
	RWOverlappedData	m_olpWrite;
	RWOverlappedData	m_olpRead;		
	IOBlock				m_RecvBlock;	//用于接收一次数据的buffer
	IOBuffer			m_RecvBuffer;   //用于存储所有的接收数据
	IOBuffer			m_SendBuffer;	//用于存储所有要发送的数据
	time_t				m_tmLastHeart;	//连接的最后一次心跳
	Mutex				m_mutex;
	volatile int		m_reference;	//连接被占用的次数
	volatile bool		m_bComplete;	//连接是否已经关闭
	volatile bool		m_bIsSending;
};

#endif//__2013_01_04_CLIENT_SOCKET_H__