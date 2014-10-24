#ifndef BENCHBASE_H
#define BENCHBASE_H

enum class Tests
{
	First,
	Second,
};

struct BenchBase
{
	virtual ~BenchBase() { ; }
	bool run(PBYTE start, DWORD size)
	{
		mBase = start;
		mSize = size;
		memset(start, 0, size);
		__try {
			init(Tests::First);
			runOne(start, size);		// see if it stops nicely at the end.
			*(start + size-1) = szPattern1[0];
			runOne(start, size);		// see if it stops nicely while matching a signature
			*(start + size-1) = 0;
			runPatt(Tests::First, start + size - 100, szPattern1, _countof(szPattern1) - 1, 7);
			runPatt(Tests::Second, start + size - 50, szPattern2, _countof(szPattern2) - 1, 11);
			return true;
		} __except (1)
		{
			std::cout << "ran outside the area" << std::endl;
		}
		return false;
	}
	bool runPatt(Tests test, PBYTE patternTarget, const char* patt, size_t len, size_t offset)
	{
		memcpy(patternTarget, patt, len);
		*reinterpret_cast<PDWORD>(patternTarget + offset) = 0x11223344;
		init(test);
		mTimer.start();
		for (int n = 0; n < kNumRuns; ++n) {
			if (runOne(mBase, mSize) != patternTarget) {
				mTimer.stop();
				std::cout << "Failed to find pattern." << std::endl;
				memset(patternTarget, 0, len);
				return false;
			}
		}
		mTimer.stop();
		double result = mTimer.milli_hp();
		std::cout << "Finding pattern " << (int)test << " x " << kNumRuns << " took " << result << " ms." << std::endl;
		memset(patternTarget, 0, len);
		return true;
	}

	virtual void init(Tests test) = 0;
	virtual LPVOID runOne(PBYTE baseAddress, DWORD size) = 0;
	virtual const char* name() const = 0;

private:
	Timer mTimer;
	PBYTE mBase;
	DWORD mSize;
};

#endif // BENCHBASE_H
