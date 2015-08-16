#include "time_value.h"
#include "common.h"

TimeValue::TimeValue()
{

}
TimeValue::TimeValue(int32_t sencond, int32_t msec /*= 0*/, int32_t usec /*= 0*/)
{
	set(sencond, msec, usec);
}

TimeValue::TimeValue(const timeval& tmval)
{
	m_tmVal = tmval;
}

TimeValue::~TimeValue()
{

}

void TimeValue::normalize()
{
	m_tmVal.tv_sec += m_tmVal.tv_usec / ONE_SECOND_TO_USECOND;
	m_tmVal.tv_usec %= ONE_SECOND_TO_USECOND;
	
	if (m_tmVal.tv_sec > 0 && m_tmVal.tv_usec < 0)
	{
		m_tmVal.tv_sec--;
		m_tmVal.tv_usec += ONE_SECOND_TO_USECOND;
	}
}

int32_t TimeValue::sec()
{
	return m_tmVal.tv_sec + m_tmVal.tv_usec / ONE_SECOND_TO_USECOND;
}

int32_t TimeValue::msec()
{
	return m_tmVal.tv_sec * ONE_SECOND_TO_MSECOND + m_tmVal.tv_usec / ONE_MSECOND_TO_USECOND;
}

int32_t TimeValue::usec()
{
	return m_tmVal.tv_sec * ONE_SECOND_TO_USECOND + m_tmVal.tv_usec;
}

void TimeValue::set(int32_t sec, int32_t msec /*= 0*/, int32_t usec /*= 0*/)
{
	m_tmVal.tv_sec = sec;
	m_tmVal.tv_usec = usec + msec * ONE_MSECOND_TO_USECOND;

	normalize();
}

#if (CUR_PLATFROM == PLATFORM_UNKNOW)
struct timeval* TimeValue::getTimeval() const
{
	TimeValue* pThis = const_cast<TimeValue*>(this);
	return &pThis->m_tmVal;
}
#endif

// TimeValue::operator struct timeval const()const
// {
// 	return m_tmVal;
// }
// 
// TimeValue::operator struct timeval const*() const
// {
// 	return &m_tmVal;
// }

TimeValue& TimeValue::operator+(const TimeValue& rTime)
{
	this->set(m_tmVal.tv_sec + rTime.m_tmVal.tv_sec,
			0,
			m_tmVal.tv_usec + rTime.m_tmVal.tv_usec);

	return *this;
}

TimeValue& TimeValue::operator-(const TimeValue& rTime)
{
	this->set(m_tmVal.tv_sec - rTime.m_tmVal.tv_sec,
		0,
		m_tmVal.tv_usec - rTime.m_tmVal.tv_usec);

	return *this;
}
