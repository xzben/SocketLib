#ifndef __2013_12_29_CONFIG_H__
#define __2013_12_29_CONFIG_H__

#include <assert.h>

/*
*	系统通用宏
*/
#define	SAFE_DELETE(_ptr)	{ if(NULL != _ptr) { delete _ptr; _ptr = NULL;} }
#define VERTIFY(_express)	if( !(_express) ) {	assert(false); }	
#endif//__2013_12_29_CONFIG_H__