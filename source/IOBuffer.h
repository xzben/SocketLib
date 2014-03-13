#ifndef __2014_03_02_CIOBUFFER_H__
#define __2014_03_02_CIOBUFFER_H__

#include "CObject.h"
#include "MemoryPool.h"

class IOBlock : public AllocFromMemoryPool, public NoCopyable
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

class IOBuffer : public NoCopyable
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
#endif//__2014_03_02_CIOBUFFER_H__