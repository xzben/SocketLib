#ifndef __2015_07_25_PACKAGE_H__
#define __2015_07_25_PACKAGE_H__

#include "common.h"
#include "Ref.h"
typedef  short	PackageSize;

class Package
{
#if (CUR_PLATFROM == PLATFORM_WIN32)
	friend class IOCPDriver;
#else
	friend class EpollDriver;
#endif
	friend class CWatchDog;
	friend class CSocketDriver;
public:
	enum Status : char
	{
		ST_INVALID, //当前无任何数据
		// 填充状态
		ST_HEAD,    //已经填充了包头（包大小） 数据
		ST_MORE,    //已经填充了包头，部分包体数据
		ST_COMPELETE, //已经将包填充完整
		ST_ERROR,	//包填充出问题

		//读取状态
		ST_SEND_HEAD,
		ST_SEND_BODY,
		ST_SEND_DONE,
	};

private:
	PackageSize	m_head;
	PackageSize	m_curSize;
	Status		m_status;
	void*		m_buffer;
public:
	static Package* create(const PackageSize bufSize = 0, void *buf = nullptr)
	{
		return new Package(bufSize, buf);
	}
	static void free(Package*& lp)
	{
		SAFE_DELETE(lp);
	}

	static void* createBuffer(PackageSize size)
	{
		return new char[size];
	}

	Package();
	Package(const PackageSize bufSize, void *buf);
	~Package();

public:
	PackageSize& getHead() { return m_head; }
	void*		 getBuffer() { return m_buffer; }
protected:
	bool isFillComplete();
	bool isSendComplete();
	//接受包时获取buffer
	bool getFillData(PackageSize& size, void*& buf);
	bool FillData(PackageSize& size, void* buf = nullptr);

	bool getSendData(PackageSize& size, void*& buf);
	bool offsetData(PackageSize size);

};

#endif//__2015_07_25_PACKAGE_H__
