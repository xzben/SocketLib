#include "package.h"
#include <cstring>

Package::Package()
: m_head(0)
, m_curSize(0)
, m_status(ST_INVALID)
, m_buffer(nullptr)
{

}

Package::Package(const PackageSize bufSize, void *buf)
{
	if (nullptr == buf || bufSize == 0)
	{
		Package();
	}
	else
	{
		m_head = bufSize;
		m_curSize = bufSize;
		m_status = ST_COMPELETE;
		m_buffer = buf;
	}
}

Package::~Package()
{
	char* tmp = (char*)m_buffer;
	SAFE_DELETE(tmp);
	m_buffer = nullptr;
}

bool Package::isFillComplete()
{
	return m_status == ST_COMPELETE;
}

bool Package::isSendComplete()
{
	return m_status == ST_SEND_DONE;
}

bool Package::getFillData(PackageSize& size, void*& buf)
{
	buf = nullptr;
	size = 0;

	switch (m_status)
	{
		case Package::ST_INVALID:
		{
			buf = &m_head;
			size = sizeof(m_head);
			return true;
		}
		case Package::ST_HEAD:
		{
			if(m_buffer == nullptr)
				m_buffer = createBuffer(m_head);
			buf = m_buffer;
			size = m_head;
			return true;
		}
		case Package::ST_MORE:
		{
			ASSERT(m_buffer != nullptr);
			buf = (char*)m_buffer + m_curSize;
			size = m_head - m_curSize;
			return true;
		}
	}

	return false;
}

bool Package::FillData(PackageSize& size, void* buf/* =nullptr */)
{
	switch (m_status)
	{
		case Package::ST_INVALID:
		{
			if (size != sizeof(m_head))
				return false;
			if (buf != nullptr)
				m_head = (*(PackageSize*)buf);
			m_status = ST_HEAD;
			return true;
		}
		case Package::ST_HEAD:
		{
			if(m_buffer == nullptr)
				m_buffer = createBuffer(m_head);
			
			m_curSize += size;
			if (m_curSize == m_head)
			{
				m_status = ST_COMPELETE;
			}
			else if (m_curSize < m_head)
			{
				m_status = ST_MORE;
			}
			else
			{
				m_status = ST_ERROR;
				return false;
			}
			if (buf != nullptr)
				memcpy(m_buffer, buf, size);

			return true;
		}
		case Package::ST_MORE:
		{
			ASSERT(m_buffer != nullptr);
			PackageSize oldSize = m_curSize;
			m_curSize += size;
			if (m_curSize == m_head)
			{
				m_status = ST_COMPELETE;
			}
			else if (m_curSize < m_head)
			{
				m_status = ST_MORE;
			}
			else
			{
				m_status = ST_ERROR;
				return false;
			}
			if (buf != nullptr)
				memcpy((char*)m_buffer + oldSize, buf, size);

			return true;
		}
	}
	
	return false;
}

bool Package::getSendData(PackageSize& size, void*& buf)
{
	switch (m_status)
	{
		case Package::ST_COMPELETE: //首先发送 head
		{
			buf = &m_head;
			size = sizeof(m_head);
			m_curSize = 0;
			m_status = ST_SEND_HEAD;
			return true;
		}
		case Package::ST_SEND_HEAD:  //head 未发送完则会继续来此发送剩余的head
		{
			buf = (char*)&m_head + m_curSize;
			size = sizeof(m_head) - m_curSize;
			m_status = ST_SEND_HEAD;
			return true;
		}
		case Package::ST_SEND_BODY:
		{
			buf = (char*)m_buffer + m_curSize;
			size = m_head - m_curSize;
			m_status = ST_SEND_BODY;
			return true;
		}
	}

	return false;
}
bool Package::offsetData(PackageSize size)
{
	switch (m_status)
	{
		case Package::ST_SEND_HEAD:
		{
			m_curSize += size;
			if (m_curSize == sizeof(m_head)) //如果head 发送完毕则切换到发送body
			{
				m_curSize = 0;
				m_status = ST_SEND_BODY;
			}
			else //如果head 未发送完毕则继续发送head
				m_status = ST_SEND_HEAD;

			return true;
		}
		case Package::ST_SEND_BODY:
		{
			m_curSize += size;

			if (m_curSize == m_head)
				m_status = ST_SEND_DONE;
			else
				m_status = ST_SEND_BODY;

			return true;
		}
	}

	return false;
}
