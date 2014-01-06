#ifndef __2013_12_29_SVR_ENGINE_H__
#define __2013_12_29_SVR_ENGINE_H__

#include <string>
#include <intsafe.h>
#include <ctime>
#include<Winsock2.h>
#include <MSWSock.h>
#include <vector>
#include <queue>
#include <assert.h>
#include "config.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment ( lib, "mswsock.lib")


class ClientSocket;
/*
*	私有化复制构造函数和复制函数
*	使子类对象无法复制
*/
class NoCopyable
{
protected:
	NoCopyable(){};
	~NoCopyable(){};
private:
	NoCopyable(const NoCopyable& );
	NoCopyable& operator=(const NoCopyable&);
};
/*
*	线程资源互斥锁
*/
class Mutex : public NoCopyable
{
public:
	Mutex()
	{
		InitializeCriticalSection( &m_csLock );
	}
	~Mutex()
	{
		DeleteCriticalSection( &m_csLock );
	}
	void	lock()
	{
		EnterCriticalSection( &m_csLock );
	}
	void	unlock()
	{
		LeaveCriticalSection( &m_csLock );
	}
private:
	CRITICAL_SECTION m_csLock;
};

/*
*	自动锁，利用局部变量的声明周期，自动调用解锁功能
*/
class AutoLock : public NoCopyable
{
public:
	AutoLock(Mutex* pMutex)
		:m_pMutex(pMutex),
		m_bLocked(false)
	{
		this->lock();
	}

	~AutoLock()
	{
		this->unlock();
	}
	
	void lock()
	{
		if(m_pMutex && !m_bLocked)
		{
			m_pMutex->lock();
			m_bLocked  = true;
		}
	}

	void unlock()
	{
		if(m_pMutex && m_bLocked)
		{
			m_pMutex->unlock();
			m_bLocked = false;
		}
	}
private:
	Mutex*	m_pMutex;
	bool	m_bLocked;
};

/*
*	线程封装
*/
class Thread : public NoCopyable
{
public:
	Thread(std::string strName, bool bJoinable = true)
		:m_strThreadName(strName),
		m_bComplete(false),
		m_bActive(false),
		m_bJoinable(bJoinable)
	{
		m_hThreadHandle = NULL;
	}
	virtual ~Thread()
	{
		if(NULL != m_hThreadHandle)
		{
			::CloseHandle(m_hThreadHandle);
		}
	}
	//线程睡眠 单位为秒
	void	sleep(const long sec)	const	{ ::Sleep( sec*1000 );	}
	//线程睡眠 单位为毫秒
	void	msleep(const long msec)	const	{ ::Sleep( msec );		}
	//线程睡眠，单位为微妙
	void	usleep(const long usec) const	{ ::Sleep( usec/1000 ); }
	//判断线程是否已经激活
	bool	isActive()				const	{ return m_bActive;		}
	//线程是否结束
	bool	isFinal()				const	{ return m_bComplete;	}
	//结束线程
	void	final()							{ m_bComplete = true;	}
	//线程是否可Join 
	bool	isJoinable()			const	{ return m_bJoinable;	}
	//线程启动
	bool	start();
	//等价线程结束，与调用线程合并
	void	join();
	//线程执行内容的回调函数
	virtual bool run() = 0;
	//获取线程的名字
	const std::string&	getThreadName()const{ return m_strThreadName; }
protected:
	static DWORD __stdcall ThreadFunc(void *arg);
protected:
	std::string			m_strThreadName; // 线程名
	Mutex				m_ThreadMutex;	// 线程互斥锁
	volatile bool		m_bComplete;	 // 线程结束开关	
	volatile bool		m_bActive;		 // 线程运行标志
	HANDLE				m_hThreadHandle; // 线程句柄
	bool				m_bJoinable;	 // 线程是否合并	
};



enum MemSize
{
	SIZE1K = 1024,
	SIZE1M = 1024 * SIZE1K,
	SIZE1G = 1024 * SIZE1M,
};

typedef unsigned int	uint32;

//basic memory node structure
struct apr_memnode_t{
	uint32			magic;			//用于标记这个内存块是内存池申请的			    
	apr_memnode_t	*next;			//指向下一个内存空间节点
	apr_memnode_t	**ref;			//指向当前内存空间节点
	uint32			index;			//当前内存空间节点的总共内存大小
	uint32			free_index;		//当前内存空间中的可用空间
	char*			first_avail;	//指向当前可用空间的起始地址　　	　　　
	char*			endp;			//指向当前可用地址的结束地址　
};

class Allocator
{
public:
	enum{
		APR_ALLOCATOR_MAX_FREE_UNLIMITED = 0,
		DEFAULT_ALIGN = 8,
		MAX_INDEX = 20,
		BOUNDARY_INDEX = 4, //决定分配器最小的分配单元大小使用时都是对 size >> BOUNDARY_INDEX 所以它代表4K的大小，其大小是分配器的大小基础单位
		BOUNDARY_SIZE =  (1 << BOUNDARY_INDEX), // 4K 分配的大小单位
		MIN_ALLOC = 2*BOUNDARY_SIZE,
	};
public:
	Allocator(size_t nMaxSize = APR_ALLOCATOR_MAX_FREE_UNLIMITED);
	virtual ~Allocator();
	inline const int GetMemNodeSize()
	{
		const int nMemNodeSize = Align(sizeof(apr_memnode_t), DEFAULT_ALIGN);
		return nMemNodeSize;
	}
	/*
	*	获得当前分配器分配的内存块的标记值
	*/
	inline uint32 GetMagic()
	{
		return m_uiMagic;
	}
	/*
	*	获得 nAllocSize 空间大小的节点
	*/
	apr_memnode_t*  Alloc(size_t nAllocSize);
	/*
	*	释放node节点的空间，注意这里的释放不一定会直接给系统回收
	*	可能是暂时留在分配器中，给下次要用的内存使用
	*/
	void Free(apr_memnode_t *node);
private:
	/*
	*	生成一个较大的随机数字
	*/
	static inline uint32	CreateMagic()
	{
		double start = 1, end = RAND_MAX;
		double uiMagic = (start + (end - start)*rand()/(RAND_MAX+1.0));
		uiMagic *= uiMagic;
		return (uint32)uiMagic;
	}
	/*
	*	function:	计算最接近nSize 的 nBoundary 的整数倍的整数，获得按指定字节对齐后的大小
	*	parameter:	nSize 为整数， nBoundary，必须为 2 的倍数
	*	example:	Align(7， 4) = 8，Align(21, 16) = 32
	*/
	static inline size_t Align(size_t nSize, size_t nBoundary)
	{
		return ((nSize +nBoundary-1) & ~(nBoundary - 1));
	}
	/*
	*	function:	设置分配子的最大内存分配空间限制，此设置关系到，
	*				当分配子中有多大内存时会将内存返回给系统回收
	*	paramter:	allocator : 要设置的分配子， nSize： 要设置的大小
	*	
	*/
	void inline SetMaxSize(size_t nSize)
	{
		//计算按 4K 对齐的空间大小后，获得其大小的4K倍数值
		uint32 uiMaxIndex = Align(nSize, BOUNDARY_SIZE) >> BOUNDARY_INDEX;
		
		//设置新的最大可存放空间大小，这操作要保证当前 m_uiCurAllocIndex(当前可存储在分配器中的内存大小)
		//做合理的调整，如果设置新最大值时，m_uiCurAllocIndex ==  m_uiMaxIndex 则要做相应的增加，
		//如果 m_uiCurAllocIndex < m_uiMaxIndex 那么加上这个差值也不会影响，因为 m_uiCurAllocIndex 会在后续的使用中
		//达到这个值。
		m_uiCurAllocIndex += uiMaxIndex - m_uiMaxIndex;
		m_uiMaxIndex = uiMaxIndex;

		if(m_uiCurAllocIndex > uiMaxIndex)
			m_uiCurAllocIndex = uiMaxIndex;
	}
	/*
	*	将分配器中挂载的空间全部给系统回收
	*/
	void Destroy();
private:
	uint32			m_uiMagic; //用于记录次分配器分配的内存块的标记值
	uint32			m_uiCurMaxBlockIndex; //分配器中当前可用的最大块的的大小index
	uint32			m_uiMaxIndex;//分配器可以存储的最大空间大小index
	uint32			m_uiCurAllocIndex;//当前已经分配的可留在分配器中的空间大小，其值总是在 m_uiMaxIndex范围内
	Mutex			*m_mutex;		 //多线程访问锁
	apr_memnode_t	*m_pfree[MAX_INDEX];//分配器当前挂载的可用内存块
};

class MemoryPool
{
public:
	MemoryPool(size_t nMaxSize = 0);
	virtual ~MemoryPool();
	static MemoryPool*	getInstance()
	{
		static MemoryPool Instance;
		return &Instance;
	}
	virtual void* Alloc(size_t nAllocaSize);
	virtual bool Free(void* pMem);
private:
	Allocator	*m_pAllocator;
	Mutex		m_mutex;
};


class AllocFromMemoryPool
{
public:
	inline void*	operator new(size_t size)
	{
		return MemoryPool::getInstance()->Alloc(size);
	}
	inline void*	operator new[](size_t size)
	{
		return MemoryPool::getInstance()->Alloc(size);
	}
	inline void	operator delete(void *pobj)
	{
		MemoryPool::getInstance()->Free(pobj);
	}
	inline void	operator delete[](void *pobj)
	{
		MemoryPool::getInstance()->Free(pobj);
	}
	virtual ~AllocFromMemoryPool()
	{

	}
protected:  //讲构造函数定义为保护类型，使此类不能被直接声明对象，只能被继承使用
	AllocFromMemoryPool()
	{

	}
};
/*
*	Socket 类封装了Socket常见的一些操作
*/
class TCPSocket : public NoCopyable
{
public:
	static bool			s_loadSockLib(int nHigh = 2, int nLow = 2);
	static bool			s_destroySockLib();
	static sockaddr_in	s_getSockAddrIpV4(const char* szIpAddr, const u_short usPort);
	static void			s_getSockAddrIpV4(const sockaddr_in addr, std::string &szIpAddr,  u_short &usPort);
private:
	static	bool		s_bLoadedSockLib;
public:
	TCPSocket();
	virtual ~TCPSocket();
	//初始化套接字
	bool	init(const int nAf = AF_INET, const int nProtocl = 0);
	//获取套接字绑定的本地地址
	bool	getLocalAddr(std::string& strIpAddr, u_short& usPort);
	//获取连接成功后的套接字的连接对方的地址
	bool	getPeerAddr(std::string& strIpAddr, u_short& usPort);
	//人工指定套接字的连接对方的地址
	bool	setPeerAddr(const sockaddr_in& addrPeer);
	bool	attach(SOCKET hSocket);
	SOCKET	dettach();
	bool	bind(const sockaddr_in& addrBind);
	bool	listen(int nBacklog = 10);
	//将套接字绑定到指定端口并启动监听服务
	bool	startServer(int nPort);
	TCPSocket*	accept();
	bool	connect(const sockaddr_in& addrCon);
	int		recv(char* pBuf, int nLen, int nFlag =0);
	int		send(const char *pBuf,const int nLen, int nFlag = 0);
	SOCKET	getSocket();
	bool	initPeerAddress();
	//使用IOCP 时需要在accept后通过此函数更新连接套接字的属性
	bool	updateAcceptContext();
	bool	shutdown(int how = SD_BOTH);
	bool	close();
	bool	disConnect();
protected:
	SOCKET			m_hSocket;	//套接字句柄
	sockaddr_in		m_addrPeer; //套接字连接后，与之连接的地址
	bool			m_bHaveAddr;//是否包含连接对象的地址
};

class IOBlock : public AllocFromMemoryPool
{
public:
	enum	{MAX_BLOCK_SIZE = 8192 }; //block 块的大小值 byte为单位
	IOBlock()
	{
		reset();
	}
	~IOBlock()
	{

	}
	void	lock()
	{
		m_mutex.lock();
	}
	void	unlock()
	{
		m_mutex.unlock();
	}
	/*
	*	读取IOBlock中的内容
	*	@para	pBuffer: 存储读取的内容，可以为NULL，也就是此次读取只是为了删除Block中的数据
	*	@para	nLenght: 要读取内容的长度
	*	@para	bDel   : 是否从Block中删除读取的内容
	*	@return:	成功读取的内容长度，如果pBuffer为NULL则此长度为读取长度的负数
	*/
	int	readData(void * pBuffer, int nLength, bool bDel = true)
	{
		if(nLength <= 0)	return 0;

		int nReadAbleSize = m_nWriteIndex - m_nReadIndex;

		if(nLength > nReadAbleSize )
			nLength = nReadAbleSize;

		if(NULL != pBuffer)
			memcpy(pBuffer, &m_pBuffer[m_nReadIndex], nLength);
		if(bDel)
			m_nReadIndex += nLength;

		return (NULL == pBuffer) ? - nLength : nLength;
	}
	/*
	*	向IOBlock中写入内容
	*	@para	pBuffer: 要写入的内容，可以为NULL，也就是此次写入只是为了从block中预留一段空间
	*	@para	nLenght: 要写入内容的长度
	*	@return:	成功写入的内容长度，如果pBuffer为NULL则此长度为写入长度的负数
	*/
	int	writeData(void* pBuffer, int nLength)
	{
		if(nLength <= 0)  return 0;

		int nWriteAbleSize = MAX_BLOCK_SIZE - m_nWriteIndex;
		if(nLength > nWriteAbleSize)
			nLength = nWriteAbleSize;

		if(NULL != pBuffer)
			memcpy(&m_pBuffer[m_nWriteIndex], pBuffer, nLength);
		m_nWriteIndex += nLength;
		
		return (NULL == pBuffer) ? -nLength : nLength;
	}
	/*
	*	获取当前Block中未被使用的buffer
	*	@para	pBuffer: 指向可用buffer的首地址
	*	@para	nLenght: 可用空间的size
	*	@return:	有可用空间返回true，否则返回false
	*/
	bool	getEmptyBuffer(void *&pBuffer, int &nDataSize)
	{
		nDataSize = MAX_BLOCK_SIZE - m_nWriteIndex;
		if(nDataSize <= 0) return false;
		pBuffer = (void*)&m_pBuffer[m_nWriteIndex];

		return true;
	}
	/*
	*	获取当前block还可以写入的buffer长度
	*	@return:	返回可写入的长度
	*/
	int		getWriteAbleSize()
	{
		return MAX_BLOCK_SIZE - m_nWriteIndex;
	}
	/*
	*	获取当前block中的已经写入数据的buffer
	*	@para	pBuffer: block中写入数据的buffer首地址
	*	@para	nDataSize: block中已经写入数据的buffer的size
	*	@return:	block中有写入数据返回true，否则返回false
	*/
	bool	getDataBuffer(void *&pBuffer, int &nDataSize)
	{
		nDataSize = m_nWriteIndex - m_nReadIndex;
		if(nDataSize <= 0) return false;
		pBuffer = (void*)&m_pBuffer[m_nReadIndex];
		return true;
	}
	/*
	*	获取当前block中的可以读取的数据size
	*	@return:	block中可读取数据的size
	*/
	int		getReadAbleSize()
	{
		return m_nWriteIndex - m_nReadIndex;
	}
	/*
	*	重置
	*/
	void	reset()
	{
		m_nWriteIndex = 0;
		m_nReadIndex = 0;
	}
private:
	Mutex			m_mutex;
	char			m_pBuffer[MAX_BLOCK_SIZE];
	int				m_nWriteIndex; //block 当前的可写入位置
	int				m_nReadIndex;  //block 当前的可读取位置
};

class IOBuffer
{
public:
	typedef		IOBlock							IOBlockTemplate;
	typedef		std::vector<IOBlockTemplate*>	IOBufferQueue;

	IOBuffer()
	{
		m_pCurBlock = NULL;
		m_nDataSize = 0;
	}

	virtual ~IOBuffer()
	{
		clearIOBlocks();
	}
	void lock()
	{
		m_mutex.lock();
	}
	void unlock()
	{
		m_mutex.unlock();
	}
	int readData(void* pBuffer, int nLength, bool bDel = true)
	{
		if( nLength <= 0 ) return 0;

		void* pDataBuf = pBuffer;
		int   nReadLen = 0;

		if(nLength > m_nDataSize)
			nLength = m_nDataSize;

		IOBufferQueue::iterator itBlock = m_IoBlocks.begin();
		while( itBlock != m_IoBlocks.end() )
		{
			IOBlockTemplate* pBlock = *itBlock;

			assert(NULL != pBlock);

			if( (NULL != pBlock) )
			{
				int nSize = pBlock->readData(pDataBuf, nLength - nReadLen, bDel);
				if(NULL != pDataBuf)
					pDataBuf = (char*)pDataBuf + nSize;
				else
					nSize *= -1;

				nReadLen += nSize;
				if(bDel)
					m_nDataSize -= nSize;

				if(nReadLen >= nLength) break;

				if(bDel)
				{
					itBlock = m_IoBlocks.erase(itBlock);
					if(m_pCurBlock == pBlock)
						m_pCurBlock = NULL;
					releaseBlock(pBlock);
				}
				else
					itBlock++;
			}
		}
		return NULL == pBuffer ? -nReadLen : nReadLen;
	}
	bool getDataBuffer(void *&pBuffer, int &nDataSize)
	{
		pBuffer = NULL; nDataSize = 0;

		IOBufferQueue::iterator itBlock = m_IoBlocks.begin();
		while( itBlock != m_IoBlocks.end() )
		{
			IOBlockTemplate *pBlock = *itBlock;
			assert(NULL != pBlock);
			if( pBlock->getDataBuffer(pBuffer, nDataSize) )
				return true;

			itBlock = m_IoBlocks.erase(itBlock);
			if(pBlock == m_pCurBlock)
				m_pCurBlock = NULL;
			releaseBlock(pBlock);
		}
		return false;
	}

	bool getEmptyBuffer(void *&pBuffer, int &nDataSize, bool bNewBlock = false)
	{
		pBuffer = NULL; nDataSize = 0;

		if(bNewBlock)
			m_pCurBlock = createBlock();
		else
		{
			if(NULL == m_pCurBlock || m_pCurBlock->getWriteAbleSize() <= 0)
				m_pCurBlock = createBlock();
		}

		return m_pCurBlock->getEmptyBuffer(pBuffer, nDataSize);
	}
	int writeData(void* pBuffer, int nLength)
	{
		if( nLength <= 0 ) return 0;

		void* pDataBuf = pBuffer;
		int   nWriteLen = 0;

		if( NULL == m_pCurBlock)
			m_pCurBlock = createBlock();

		while(1)
		{
			int nSize = m_pCurBlock->writeData(pDataBuf, nLength - nWriteLen);
			if(NULL != pDataBuf)
				pDataBuf  = (char*)pDataBuf + nSize;
			else
				nSize *= -1;

			nWriteLen += nSize;
			m_nDataSize += nSize;

			if(nWriteLen >= nLength) break;

			m_pCurBlock = createBlock();
		}

		return NULL == pBuffer ? -nWriteLen : nWriteLen;
	}

	int	 getDataSize()
	{
		return m_nDataSize;
	}
private:
	void clearIOBlocks()
	{
		IOBufferQueue::iterator it = m_IoBlocks.begin();
		for(; it != m_IoBlocks.end(); it++)
		{
			IOBlockTemplate *pBlock = *it;
			releaseBlock(pBlock);
		}
		m_IoBlocks.clear();		
	}
	void	releaseBlock(IOBlockTemplate *&pBlock)
	{
		SAFE_DELETE(pBlock);
	}
	IOBlockTemplate* createBlock()
	{
		IOBlockTemplate* pNewBlock = new IOBlockTemplate;
		m_IoBlocks.push_back(pNewBlock);
		return pNewBlock;
	}
private:
	IOBufferQueue		m_IoBlocks;
	int					m_nDataSize;
	IOBlockTemplate	   *m_pCurBlock;
	Mutex				m_mutex;
};

template<typename	_TaskType>
class TaskQueue : public NoCopyable
{
public:
	TaskQueue():m_nSize(0){}
	virtual ~TaskQueue(){}

	void	addTask(const _TaskType& pTask)
	{
		AutoLock lock(&m_TaskQueuemutex);
		m_taskQueue.push(pTask);
		m_nSize ++;
	}
	bool	checkQueue()
	{
		AutoLock lock(&m_TaskQueuemutex);
		while(!m_taskQueue.empty())
		{
			_TaskType& task = m_taskQueue.front();
			_addTask(task);
			m_taskQueue.pop();
		}
		m_nSize = 0;
		return true;
	}
protected:
	virtual void	_addTask(_TaskType pTask) = 0;
	int				m_nSize;
protected:
	Mutex				m_TaskQueuemutex;
	typedef std::queue<_TaskType>	QueueContainer;
	QueueContainer		m_taskQueue;
};


/*
*	系统日志，按天为单位存储日志文件
*	可以设置日志显示等级, 具体等级信息看类中的 LOG_LEVEL 枚举
*/
class Logger : public NoCopyable
{
public:
	/*
	*	日志等级, 通过日志等级可以设置日志中显示的信息
	*/
	typedef enum
	{
		/*
		*	关闭日志功能，将不会显示任何信息
		*/
		LEVEL_OFF = INT_MAX,
		/*
		*	设置为LEVEL_FATAL时，显示等于此等级的信息
		*	致命错误: 此类错误是将导致系统崩溃的
		*/
		LEVEL_FATAL = 500000,
		/*
		*	设置为LEVEL_ERROR时，显示等级大于等于此等级的信息
		*	错误:	此类为系统中一些错误，但是系统还能正常运行
		*/
		LEVEL_ERROR = 400000,		   //	错误等级，显示
		/*
		*	设置为LEVEL_WARN时，显示等级大于等于此等级的信息
		*	警告: 系统中一些需要注意的地方, 但是还不属于错误，但是可能是后面错误产生的根源
		*/
		LEVEL_WARN  = 300000,
		/*
		*	设置为LEVEL_INFO时，显示等级大于等于此等级的信息
		*	信息: 系统中一些的一些运行信息，用于跟踪服务器的状态
		*/
		LEVEL_INFO  = 20000,
		/*
		*	设置为LEVEL_DEBUG时，显示全部的信息
		*/
		LEVEL_DEBUG = 10000,
		/*
		*	设置为LEVEL_ALL时，显示全部的信息
		*/
		LEVEL_ALL = INT_MIN, 
	}LOG_LEVEL;
	
	~Logger(); 
	static Logger* getInstace()
	{
		static Logger instance;
		return &instance;
	}
	void removeConsoleLog();
	void addLocalFileLog(const std::string &file);

	void setLevel(const LOG_LEVEL level);
	void setLevel(const std::string &level);

	void logtext(const LOG_LEVEL level,const char * text);
	void logva(const LOG_LEVEL level,const char * pattern,va_list vp);
	void log(const LOG_LEVEL level,const char * pattern,...);

	void debug(const char * pattern,...);
	void debug16(const char* info, const BYTE* pData, int Datasize);
	void error(const char * pattern,...);
	void info(const char * pattern,...);
	void fatal(const char * pattern,...);
	void warn(const char * pattern,...);
	void setLoggerName(std::string strLogNmae){ m_strLogName = strLogNmae; }
protected:
	Logger(std::string strLogName = "LOGGER", LOG_LEVEL emLevel= LEVEL_ERROR);
private:
	Mutex			m_mutex;
	LOG_LEVEL		m_emLevel;
	std::string		m_strLogName;
	std::string		m_file;
	FILE			*m_fp_console;
	FILE			*m_fp_file;		
	int				m_day;			//当前打开日志的是哪天的，每天一个日志文本
};
#endif//__2013_12_29_SVR_ENGINE_H__