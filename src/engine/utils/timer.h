#ifndef __YOCTO_TIMER__
#define __YOCTO_TIMER__

#include <windows.h>

class YTimer
{
public:
	LARGE_INTEGER lastUpdateTime;
	LONGLONG freq;
	LONGLONG AccumulatedTime = 0;
	LARGE_INTEGER PeriodStart;
	LARGE_INTEGER PeriodEnd;


	YTimer()
	{
		QueryPerformanceCounter(&lastUpdateTime);
		LARGE_INTEGER li_freq;
		QueryPerformanceFrequency(&li_freq);
		freq = li_freq.QuadPart;
		freq /= 1000;
	}

	void start(void)
	{
		QueryPerformanceCounter(&lastUpdateTime);
	}

	float getElapsedSeconds(bool restart = false)
	{
		LARGE_INTEGER timeNow;
		QueryPerformanceCounter(&timeNow);
		LONGLONG elapsedLong = timeNow.QuadPart - lastUpdateTime.QuadPart;

		float elapsed = (float)((float)elapsedLong / (float)freq);
		elapsed /= 1000.0f;

		if (restart)
			lastUpdateTime = timeNow;

		return elapsed;
	}

	unsigned long getElapsedMs(bool restart = false)
	{
		LARGE_INTEGER timeNow;
		QueryPerformanceCounter(&timeNow);
		LONGLONG elapsedLong = timeNow.QuadPart - lastUpdateTime.QuadPart;

		unsigned long elapsed = (unsigned long)((float)elapsedLong / (float)freq);
		return elapsed;
	}

	void startAccumPeriod() {
		QueryPerformanceCounter(&PeriodStart);
	}

	void endAccumPeriod() {
		QueryPerformanceCounter(&PeriodEnd);
		LONGLONG elapsedLong = PeriodEnd.QuadPart - PeriodStart.QuadPart;
		AccumulatedTime += elapsedLong;

	}

	void resetAccumPeriod() {
		AccumulatedTime = 0;
	}

	float getAccumTimeSec(){
		float elapsed = (float)((double)AccumulatedTime / (double)freq);
		elapsed /= 1000.0f;
		return elapsed;
	}
};


#endif