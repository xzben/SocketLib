/********************************************************************************
*	文件名称:	SocketDriver.h													*
*	创建时间：	2014/03/29														*
*	作   者 :	xzben															*
*	文件功能:	系统网络层的实现													*
*********************************************************************************/
#ifndef __2015_03_29_SOCKET_DRIVER_H__
#define __2015_03_29_SOCKET_DRIVER_H__

#include "SingleInstance.h"
#include "Thread.h"

class CSocketDriver : public CThread, public SingleInstance<CSocketDriver>
{
public:
	
protected:
	friend class SingleInstance<CSocketDriver>;
	CSocketDriver();
	virtual ~CSocketDriver();
	virtual void initInstance() override;
	virtual void run()override;
};
#endif//__2015_03_29_SOCKET_DRIVER_H__