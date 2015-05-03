/********************************************************************************
*	文件名称:	Atom.h														*
*	创建时间：	2014/10/21														*
*	作   者 :	xzben															*
*	文件功能:	系统原子操作功能									*
*********************************************************************************/

#ifndef __2014_10_21_ATOM_H__
#define __2014_10_21_ATOM_H__

#include <cstdint>

/*
*	function : 原子量自加 1
*	parameter: var 要被增加的原子量地址
*	return:
*		WIN32: 返回自增后的值
*/
extern inline uint32_t AtomSelfAdd(void * var);

/*
*	function : 原子量自减 1
*	parameter: var 要被操作的原子量地址
*	return:
*		WIN32: 返回自减后的值
*/
extern inline uint32_t AtomSelfDec(void * var);

/*
*	function : 原子量增加指定值
*	parameter: var 要被操作的原子量地址， value 要增加的值
*	return:
*		WIN32: 返回增加前的值
*/
extern inline uint32_t AtomAdd(void * var, const uint32_t value);

/*
*	function : 原子量减小指定值
*	parameter: var 要被操作的原子量地址， value 要减小的值
*	return:
*		WIN32: 返回减小前的值
*/
extern inline uint32_t AtomDec(void * var, int32_t value);

/*
*	function : 原子量设置成指定的值
*	parameter: var 要被操作的原子量地址， value 要被设置的值
*	return:
*		WIN32: 返回设置前的值
*/
extern inline uint32_t AtomSet(void * var, const uint32_t value);

/*
*	function : 获取原子量的值
*	parameter: var 要被操作的原子量地址
*	return:
*		WIN32: 原子量的当前值
*/
extern inline uint32_t AtomGet(void * var);

#endif//__2014_10_21_ATOM_H__