#include "socket_handle.h"


#if (CUR_PLATFROM == PLATFROM_LINUX)
int32_t 	GetLastError()
{
	return errno;
}

#endif//_LINUX
