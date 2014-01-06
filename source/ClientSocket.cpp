#include "ClientSocket.h"
#include "Acceptor.h"

volatile unsigned int ClientSocket::s_ClientSocketCount = 0;
ClientSocket::ClientSocket()
	:m_olpWrite(IO_Write),
	m_olpRead(IO_Read),
	m_reference(1),
	m_bComplete(false),
	m_bIsSending(false)
{
	s_ClientSocketCount++;
	refreshHeart();
	Logger::getInstace()->debug("new ClientSocket current Count[%d]", s_ClientSocketCount);
}

ClientSocket::~ClientSocket()
{
	s_ClientSocketCount--;
	Logger::getInstace()->debug("delete ClientSocket current Count[%d]", s_ClientSocketCount);
}
bool ClientSocket::SendData(void *pBuffer, const int nSize)
{
	if( isFinal() ) return false; // 如果连接已经结束则不需要再进行发送操作

	m_SendBuffer.lock();
	int nSendSize = m_SendBuffer.writeData(pBuffer, nSize);
	m_SendBuffer.unlock();

	if(nSendSize != nSize)
	{
		Logger::getInstace()->error("ClientSocket[%d], SendData 失败", m_hSocket);
		return false;
	}
	return addSend();
}

int	ClientSocket::ReadData(void *pBuffer,	const int nSize, bool bDel /*= true*/)
{
	m_RecvBuffer.lock();
	int nRet = m_RecvBuffer.readData(pBuffer, nSize, bDel);
	m_RecvBuffer.unlock();

	return nRet;
}

bool	ClientSocket::addSend()
{
	AutoLock lock(&m_mutex);
	if(m_bIsSending) return true; //原先发送操作还未完成
	lock.unlock();

	m_SendBuffer.lock();
	void *pDataBuffer; int nDataSize;
	m_SendBuffer.getDataBuffer(pDataBuffer, nDataSize);
	m_SendBuffer.unlock();

	if(nDataSize <= 0) return true;//没有要发送的数据了

	m_mutex.lock();
	m_bIsSending = true;
	m_mutex.unlock();
	//投递新的发送事件
	m_olpWrite = RWOverlappedData(IO_Write);
	m_olpWrite._wsaBuffer.buf = (char*)pDataBuffer;
	m_olpWrite._wsaBuffer.len = nDataSize;

	DWORD dwSend = 0;
	DWORD dwFlags = 0;

	//投递数据接收操作
	if ( SOCKET_ERROR == ::WSASend( m_hSocket, 
		&m_olpWrite._wsaBuffer,
		1, &dwSend, dwFlags, 
		&m_olpWrite.Overlapped, NULL ) )
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			Logger::getInstace()->warn("ClientSocket[%d], addSend 失败", m_hSocket);
			return false;
		}
	}
	addReference(); //连接进入 IOCPDriver 中, 加一次引用，保证其从IOCP中出来时有效
	return true;
}
bool	ClientSocket::sendout(int nSendDoneSize)
{
	m_SendBuffer.lock();
	//将发送完的数据删除掉
	m_SendBuffer.readData(NULL, nSendDoneSize);
	m_SendBuffer.unlock();
	
	m_mutex.lock();
	m_bIsSending = false;
	m_mutex.unlock();
	//获得剩下未发送的数据

	// 连接从 IOCPDriver 中出来了，减一次引用
	if( !decReference() )
	{
		return true;  //减引用后对象已经无效直接返回
	}
	return addSend();
}
bool	ClientSocket::recvDone(int nRecvDoneSize)
{
	//完成上次投递的recv操作，将接受到的内容从接受block移到接受buffer中
	m_RecvBlock.writeData(NULL, nRecvDoneSize);//完成io写操作，主要是为了移动block的写指针位置
	void *pDataBuffer = NULL; int nDataSize = 0;
	m_RecvBlock.getDataBuffer(pDataBuffer, nDataSize);//获得写入的数据，将其写入接收buffer
	m_RecvBuffer.writeData(pDataBuffer, nDataSize);	
	//////////////////////////////////////////////////////////////////////////
	//加入 IO 工作队列中
	IOWorkFactory::getInstance()->AddTask(IOTask(this, IOOperationType::IO_Read, nRecvDoneSize));
	//////////////////////////////////////////////////////////////////////////
	// 连接从 IOCPDriver 中出来了，减一次引用
	if( !decReference() )
	{
		return true;  //减引用后对象已经无效直接返回, 由于上面 的IOTask 也会对连接加引用，所以此处正常是不会执行
	}
	//投递新的recv事件
	return addRecv();
}
bool ClientSocket::addRecv()
{
	//重置overlapped
	m_olpRead = RWOverlappedData(IO_Read);

	m_RecvBlock.reset(); //重置 recv block, 并用它重新投递recv事件
	void *pBuffer; int bufferSize;
	m_RecvBlock.getEmptyBuffer(pBuffer, bufferSize);
	m_olpRead._wsaBuffer.buf = (char*)pBuffer;
	m_olpRead._wsaBuffer.len = bufferSize;

	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	//投递数据接收操作
	if ( SOCKET_ERROR == ::WSARecv( m_hSocket, 
		&m_olpRead._wsaBuffer, 
		1, &dwRecv, &dwFlags, 
		&m_olpRead.Overlapped, NULL ) )
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			return false;
			Logger::getInstace()->warn("ClientSocket[%d], addRecv 失败", m_hSocket);
		}
	}
	addReference(); //连接进入 IOCPDriver 中, 加一次引用，保证其从IOCP中出来时有效
	return true;
}