#ifndef __2015_08_12_CLUSTER_H__
#define __2015_08_12_CLUSTER_H__

#include "Server.h"
#include "SingleInstance.h"
#include "socket_handle.h"
#include <map>

class Cluster : public CServer
{
	struct NodeInfo
	{
		NodeInfo(std::string& address, SOCKET_HANDLE socket)
			: addr(address)
			, sock(socket)
		{

		}
		std::string		  addr;
		SOCKET_HANDLE	  sock;
	};
public:
	Cluster();
	virtual ~Cluster();
	bool open(const std::string& nodeName, const std::string& configFile );
protected:
	virtual void excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData) override;

private:
	typedef std::unordered_map<std::string, NodeInfo>  AddressMap;
	AddressMap		m_addrs;
	std::string		m_nodeName;
};
#endif // !__2015_08_12_CLUSTER_H__
