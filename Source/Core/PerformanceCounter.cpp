#include "PerformanceCounter.h"

#include<assert.h>
#include "string.h"

// --------------------------------------------------------------------------------
PerformanceCounter::PerformanceCounter() : m_clockCycles(-1), m_countsAtBeginning(-1)
{

}

void PerformanceCounter::BeginTiming()
{
	LARGE_INTEGER countsAtBeginning;
	QueryPerformanceCounter(&countsAtBeginning);
	m_countsAtBeginning = countsAtBeginning.QuadPart;
}

// --------------------------------------------------------------------------------
void PerformanceCounter::EndTiming()
{
	LARGE_INTEGER countsAtEnd;
	QueryPerformanceCounter(&countsAtEnd);
	m_clockCycles = countsAtEnd.QuadPart - m_countsAtBeginning;
}

// --------------------------------------------------------------------------------
void PerformanceCounter::Accumulate(LONGLONG& result)
{
	result += m_clockCycles;
}

// --------------------------------------------------------------------------------
double PerformanceCounter::GetMilliseconds() const
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency))
	{
		assert(0);
	}

	return (1000.0 * (m_clockCycles) / (double)frequency.QuadPart);
}

// --------------------------------------------------------------------------------
PerformanceCounter::~PerformanceCounter()
{

}