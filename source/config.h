/********************************************************************************
*	文件名称:	config.h														*
*	创建时间：	2015/03/29														*
*	作   者 :	xzben															*
*	文件功能:	跨平台编译支持的宏定义											*
*********************************************************************************/
#ifndef __2015_03_29_CONFIG_H__
#define __2015_03_29_CONFIG_H__

#define PLATFROM_UNKNOW		0	//未知	  系统
#define PLATFORM_WIN32		1	//Windows 系统
#define PLATFROM_LINUX		2	//Linux	  系统
#define PlATFROM_MAC		3	//Mac	  系统

#ifndef CUR_PLATFROM
	#if defined(WIN32)
		#define CUR_PLATFROM	PLATFORM_WIN32
	#elif defined(LINUX)
		#define CUR_PLATFROM	PLATFROM_LINUX
	#elif defined(MAC)
		#define CUR_PLATFROM	PlATFROM_MAC
	#else
		#define CUR_PLATFROM	PLATFROM_UNKNOW
	#endif
#endif//--!!CUR_PLATFROM

#if (CUR_PLATFROM == PLATFORM_WIN32)
	# define EXPORT_DLL												__declspec (dllexport)
	# define IMPORT_DLL												__declspec (dllimport)
	# define EXPORT_C_DLL											extern "C" __declspec(dllexport)
	# define IMPORT_C_DLL											extern "C" __declspec(dllimport)
	# define CALLBACK												__stdcall
# define __FUNCTION_NAME__											__FUNCDNAME__
#else
	# define EXPORT_DLL												
	# define IMPORT_DLL												
	# define EXPORT_C_DLL											
	# define IMPORT_C_DLL											
	# define CALLBACK
	# define WINAPI
	# define  _snprintf(buf, sz, fmt, ...) 							sprintf(buf, fmt, __VA_ARGS__)		
	# define strcpy_s(dest, sz, sc)									strcpy(dest, sc)
	# define __FUNCTION_NAME__										__func__
#endif

#if defined(SOCKETLIB_BUILD)
	#define EXP_IMP_DLL			EXPORT_DLL
	#define EXP_IMP_C_DLL		EXPORT_C_DLL
#else//--!!SOCKETLIB_BUILD
	#define EXP_IMP_DLL			IMPORT_DLL
	#define	EXP_IMP_C_DLL		IMPORT_C_DLL
#endif//--!!SOCKETLIB_BUILD

#endif//__2015_03_29_CONFIG_H__
