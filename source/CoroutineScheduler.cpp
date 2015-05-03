#include "CoroutineScheduler.h"
#include "common.h"

#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include<windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#endif

#define STACK_SIZE (64<<10)
enum
{
	COROUTINE_NONE = 0,
	COROUTINE_SUSPEND,
	COROUTINE_RUNNING,
	COROUTINE_END,
};

typedef class CoSchedulerImplement* coenv_t;

struct coroutine
{
	int state;
	coenv_t env;
	CCoroutineScheduler* sch;
	coroutine_func func;
	void *context;
#ifdef WIN32
	HANDLE fiber;
#else
	char *stack;
	int stacksize;
	ucontext_t uctx;
#endif
	struct coroutine *main;
};

typedef struct coroutine *cort_t;

#ifdef WIN32
static void WINAPI _proxyfunc(void *p)
{
	cort_t co = (cort_t)p;
#else
static void _proxyfunc(uint32_t low32, uint32_t hi32)
{
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	cort_t co = (cort_t)ptr;
#endif
	co->func(co->sch, co->context);
	co->state = COROUTINE_END;
#ifdef WIN32
	SwitchToFiber(co->main->fiber);
#endif
}

class CoSchedulerImplement
{
public:
	CoSchedulerImplement(CCoroutineScheduler* parent);
	~CoSchedulerImplement();
	
	void	resumeCoroutine(CO_ID id);
	void	yieldCoroutine();
	CO_ID	newCoroutine(coroutine_func func, void* context);
protected:
	cort_t	newMainCoroutine();
	void	deleteMainCorouitine();
	cort_t	_newCoroutine(coroutine_func func, void* context);
	void	deleteCoroutine(cort_t co);
	void	switchCoroutine(cort_t from, cort_t to);
	CO_ID	storeCoroutine(cort_t co);
private:
	friend void WINAPI _proxyfunc(void *p);
	CCoroutineScheduler* m_parent;
	int		m_nco;
	int		m_cap;
	int		m_running;
	cort_t *m_aco;
	cort_t	m_main;
};

void CoSchedulerImplement::resumeCoroutine(CO_ID id)
{
	if (0 <= id && id < this->m_cap)
	{
		cort_t co = this->m_aco[id];
		if (co && co->state == COROUTINE_SUSPEND)
		{
			this->m_running = id;
			co->state = COROUTINE_RUNNING;
			switchCoroutine(this->m_main, co);

			if (co->state == COROUTINE_END)
			{
				this->m_aco[id] = NULL;
				this->m_nco--;
				this->m_running = -1;
				deleteCoroutine(co);
			}
		}
	}
}

void	CoSchedulerImplement::yieldCoroutine()
{
	int id = this->m_running;
	if (0 <= id && id < this->m_cap)
	{
		cort_t co = this->m_aco[id];
		if (co && co->state == COROUTINE_RUNNING)
		{
			this->m_running = -1;
			co->state = COROUTINE_SUSPEND;
			switchCoroutine(co, this->m_main);
		}
	}
}

CoSchedulerImplement::CoSchedulerImplement(CCoroutineScheduler* parent)
:m_parent(parent)
{
	m_nco = 0;
	m_cap = 0;
	m_cap = 0;
	m_running = -1;
	m_aco = NULL;
	m_main = newMainCoroutine();
}

CoSchedulerImplement::~CoSchedulerImplement()
{
	for (int i = 0; i < m_cap; i++)
	{
		cort_t co = m_aco[i];
		if (co)
		{
			
		}
	}
	free(m_aco);
}

cort_t	CoSchedulerImplement::newMainCoroutine()
{
	struct coroutine *co = (struct coroutine *)malloc(sizeof(*co));
	co->state = COROUTINE_RUNNING;
	co->env = nullptr;
	co->sch = nullptr;
	co->func = nullptr;
	co->context = nullptr;

#ifdef WIN32
	co->fiber = ConvertThreadToFiber(NULL);
#endif

	return co;
}

void	CoSchedulerImplement::deleteMainCorouitine()
{
#ifdef WIN32
	ConvertFiberToThread();
#endif
	free(m_main);
}

cort_t	CoSchedulerImplement::_newCoroutine(coroutine_func func, void* context)
{
	struct coroutine *co = (struct coroutine*)malloc(sizeof(*co));
	co->state = COROUTINE_SUSPEND;
	co->env = this;
	co->sch = m_parent;
	co->func = func;
	co->context = context;
	co->main = m_main;
#ifdef WIN32
	co->fiber = CreateFiber(0, _proxyfunc, co);
#else
	co->stacksize = STACK_SIZE;
	co->stack = (char*)malloc(co->stacksize);

	getcontext(&co->uctx);
	co->uctx.uc_stack.ss_sp = co->stack;
	co->uctx.uc_stack.ss_size = co->stacksize;
	co->uctx.uc_link = &m_main->uctx;

	uintptr_t ptr = (uintptr_t)co;
	makecontext(&co->uctx, (void(*)(void))_proxyfunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
#endif
	return co;
}

void	CoSchedulerImplement::deleteCoroutine(cort_t co)
{
#ifdef WIN32
	DeleteFiber(co->fiber);
#else
	free(co->stack);
#endif
	free(co);
}

CO_ID	CoSchedulerImplement::newCoroutine(coroutine_func func, void* context)
{
	cort_t co = _newCoroutine(func, context);
	return storeCoroutine(co);
}

void	CoSchedulerImplement::switchCoroutine(cort_t from, cort_t to)
{
#ifdef WIN32
	SwitchToFiber(to->fiber);
#else
	swapcontext(&from->uctx, &to->uctx);
#endif
}

CO_ID	CoSchedulerImplement::storeCoroutine(cort_t co)
{
	if (this->m_nco >= this->m_cap)
	{
		int newcap = (this->m_cap == 0) ? 16 : this->m_cap * 2;
		this->m_aco = (cort_t *)realloc(this->m_aco, newcap * sizeof(cort_t));
		memset(this->m_aco + this->m_cap, 0, (newcap - this->m_cap) * sizeof(cort_t));
		this->m_cap = newcap;
	}

	for (int i = 0; i < this->m_cap; i++)
	{
		int id = (i + this->m_nco) % this->m_cap;
		if (this->m_aco[id] == NULL)
		{
			this->m_aco[id] = co;
			this->m_nco++;
			return id;
		}
	}
	return -1;
}
//////////////////////////////////////////////////////////////////////////
CCoroutineScheduler::CCoroutineScheduler()
{
	m_pCore = new CoSchedulerImplement(this);
}

CCoroutineScheduler::~CCoroutineScheduler()
{
	SAFE_DELETE(m_pCore);
}

CO_ID CCoroutineScheduler::newCoroutine(coroutine_func func, void* context)
{
	return m_pCore->newCoroutine(func, context);
}

void CCoroutineScheduler::resumeCoroutine(CO_ID id)
{
	m_pCore->resumeCoroutine(id);
}

void CCoroutineScheduler::yieldCoroutine()
{
	m_pCore->yieldCoroutine();
}