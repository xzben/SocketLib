/********************************************************************************
*	文件名称:	socket_addr.cpp													*
*	创建时间：	2014/03/18														*
*	作   者 :	xzben															*
*	文件功能:	本文件主要实现一个和平台无关的Socket地址类							*
*********************************************************************************/

#include "socket_addr.h"
#include <memory.h>

InterAddress InterAddress::addr_any;

InterAddress::InterAddress()
{
	open(0, nullptr);
}

InterAddress::InterAddress(int32_t Port, const char*	szIp /*= nullptr*/)
{
	open(Port, szIp);
}

InterAddress::InterAddress(sockaddr*	pSockaddr)
{
	open(pSockaddr);
}

InterAddress::~InterAddress()
{

}

void InterAddress::open(int32_t Port, const char* szIp /*=nullptr*/)
{
	memset(&m_addr, 0, sizeof(m_addr));

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(Port);
	if (nullptr == szIp)
	{
#if (CUR_PLATFROM == PLATFORM_WIN32)
		m_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#elif (CUR_PLATFROM == PLATFORM_UNKNOW)
		m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	}
	else
	{
#if (CUR_PLATFROM == PLATFORM_WIN32)
		m_addr.sin_addr.S_un.S_addr = inet_addr(szIp);
#elif (CUR_PLATFROM == PLATFORM_UNKNOW)
		m_addr.sin_addr.s_addr = inet_addr(szIp);
#endif
	}
}

void InterAddress::open(sockaddr* pSockaddr)
{
	VERIFY(sizeof(sockaddr) == sizeof(sockaddr_in));

	memcpy(&m_addr, pSockaddr, sizeof(sockaddr_in));
	//m_addr = *(sockaddr_in*)pSockaddr;
}

bool InterAddress::getAddress(char* szIp, int32_t& port)const
{
	port = ntohs(m_addr.sin_port);
	strcpy_s(szIp, 20, inet_ntoa(m_addr.sin_addr));


	return true;
}

int32_t  InterAddress::getAddrLen() const
{
	return sizeof(m_addr);
}

sockaddr*	InterAddress::getAddress() const
{
	return (sockaddr*)&m_addr;
}

