#pragma once

#include <string>
#include <windows.h>

class PerformanceCounter
{
public:

	PerformanceCounter();

	PerformanceCounter(const PerformanceCounter&) = delete;
	PerformanceCounter& operator=(const PerformanceCounter&) = delete;

	void BeginTiming();
	void EndTiming();
	void Accumulate(LONGLONG& result);
	double GetMilliseconds() const;

	~PerformanceCounter();

	int64_t m_countsAtBeginning;
	int64_t m_clockCycles;
};
