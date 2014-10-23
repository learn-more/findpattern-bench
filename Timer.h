#ifndef TIMER_H
#define TIMER_H

#ifndef LARGE_INTEGER
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

class Timer
{
public:
	Timer()
	{
		mStop.QuadPart = mStart.QuadPart = 0;
		if (!mFreq.QuadPart) {
			QueryPerformanceFrequency(&mFreq);
		}
	}

	bool running()
	{
		return mStop.QuadPart == 0 && mStart.QuadPart != 0;
	}

	void start()
	{
		mStop.QuadPart = 0;
		QueryPerformanceCounter(&mStart);
	}

	void stop()
	{
		QueryPerformanceCounter(&mStop);
		mStop.QuadPart = ((mStop.QuadPart - mStart.QuadPart) * 1000000) / mFreq.QuadPart;
	}

	const LARGE_INTEGER& nano() const
	{
		return mStop;
	}

	long milli() const
	{
		return static_cast<long>(mStop.QuadPart ? ((mStop.QuadPart + 500) / 1000) : 0);
	}

	double milli_hp() const
	{
		return mStop.QuadPart / 1000.0;
	}

	long sec() const
	{
		return static_cast<long>(mStop.QuadPart ? ((mStop.QuadPart + 500000) / 1000000) : 0);
	}

	double sec_hp() const
	{
		return mStop.QuadPart / 1000000.0;
	}

private:
	static LARGE_INTEGER mFreq;
	LARGE_INTEGER mStart, mStop;
};

__declspec(selectany) LARGE_INTEGER Timer::mFreq = { 0 };

#endif // TIMER_H
