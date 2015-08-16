#include "Cluster.h"
#include "GlobalController.h"
#include "ConfigFile.h"
#include "socket.h"
#include "SocketDriver.h"
using namespace std;

static bool getPortIp(string source, string& ip, short& port)
{
	string::size_type pos = source.find_first_of(':');
	if (pos == string::npos) return false;

	ip = source.substr(0, pos);
	string str_port = source.substr(pos + 1, source.length() - pos - 1);
	port = atoi(str_port.c_str());

	return true;
}

Cluster::Cluster()
{

}

Cluster::~Cluster()
{
	for (auto it = m_addrs.begin(); it != m_addrs.end(); it++)
	{
		if (it->second.sock != INVALID_SOCKET)
		{
			Socket tmp(it->second.sock);
			tmp.close();
		}
	}
}

bool Cluster::open(const std::string& nodeName, const std::string& configFile)
{
	m_nodeName = nodeName;

	CConfigFile config;
	config.init(configFile.c_str());

	auto valus = config.getValueMap();
	for (auto it = valus.begin(); it != valus.end(); it++)
	{
		if (it->first == m_nodeName)
			continue;

		string ip; short port;
		SOCKET_HANDLE sockhandle = INVALID_SOCKET;

		if (getPortIp(it->second, ip, port))
		{
			Socket conSock(Socket::SOCK_TCP);
			LOG_INFO("begin connect: %s [ %s | %d ]", it->second.c_str(), ip.c_str(), port);
			bool ok = conSock.connect(InterAddress(port, ip.c_str()), TimeValue(10));
			sockhandle = ok ? conSock.dettach() : INVALID_SOCKET;
			LOG_INFO("cluster connect %s : [%s]", ok ? "sucess" : "failed", it->second.c_str());
		}
		else
		{
			LOG_FATAL("cluster error address:[ %s ]", it->second);
			return false;
		}

		m_addrs.insert(AddressMap::value_type(it->first, NodeInfo(it->second, sockhandle)));
	}

	return true;

}

void Cluster::excuteOneTask(TaskType type, SESSION_ID session, SERVER_HANDLE source, CData *pData)
{

}

