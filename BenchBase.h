#ifndef BENCHBASE_H
#define BENCHBASE_H

enum class Tests
{
	First,
	Second,
};

static void memrandset(void* mem, DWORD size, unsigned int seed)
{
	srand(seed);
	for (DWORD i = 0; i < size; i++)
		((unsigned char*)mem)[i] = rand() % 256;
}

struct BenchBase
{
	virtual ~BenchBase() { ; }
	bool run(PBYTE start, DWORD size)
	{
		mBase = start;
		mSize = size;
		memrandset(start, size, GetTickCount());
		__try {
			init(Tests::First);
			runOne(start, size);		// see if it stops nicely at the end.
			*(start + size - 1) = szPattern1[0];
			runOne(start, size);		// see if it stops nicely while matching a signature
			*(start + size - 1) = 0;

			memset(start, szPattern1[0], szPattern1Len);
			memcpy(start + 5, szPattern1, szPattern1Len);
			if ((start + 5) != runOne(start, size)) {		// see if people don't cheat by skipping entire pattern forward.
				std::cout << "Failed, cheating with the pattern length!" << std::endl;
				return false;
			}
			memset(start, 0, szPattern1Len * 2);

			int o1		= (rand() % 250) + szPattern1Len + 1;
			int o2		= (rand() % 100) + szPattern2Len + 1;

			if (BackwardsSearch())
			{
				runPatt(Tests::First, start + o1, szPattern1, szPattern1Len, 7);
				runPatt(Tests::Second, start + o2, szPattern2, szPattern2Len, 11);
			}
			else
			{
				runPatt(Tests::First, start + size - o1, szPattern1, szPattern1Len, 7);
				runPatt(Tests::Second, start + size - o2, szPattern2, szPattern2Len, 11);
			}

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

	virtual bool BackwardsSearch() const
	{
		return false;
	}

private:
	Timer mTimer;
	PBYTE mBase;
	DWORD mSize;
};

#endif // BENCHBASE_H
