#include "socket_handle.h"


#if (CUR_PLATFROM == PLATFORM_UNKNOW)
int32_t 	GetLastError()
{
	return errno;
}

#endif//_LINUX
