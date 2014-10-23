#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <vector>
#include "Timer.h"

const char szPattern1[] = "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
const char szWildcard1[] = "xxxxxxx????xxx";

const char szPattern2[] = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
const char szWildcard2[] = "xxxxxxxxxxx????xxx";

enum { kNumPages = 22 };
enum { kNumRuns = 1000 };

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
			runOne(start, size);		//just to see if it stops nicely at the end.
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

std::vector<BenchBase*> tests;

size_t addTest(BenchBase* b)
{
	tests.push_back(b);
	return tests.size();
}

#define REGISTER(x)		static size_t dummy_reg = addTest( new x() );

namespace MiKe
{
#include "patterns/MiKe.h"
}
namespace Trippeh
{
#include "patterns/Trippe.h"
}
namespace learn_more
{
#include "patterns/learn_more.h"
}

int main()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	std::cout << "FindPattern benchmark" << std::endl;
	std::cout << "Page size: " << si.dwPageSize << ", allocating " << kNumPages << " pages (including 2 guard pages)." << std::endl;
	DWORD size = si.dwPageSize * kNumPages;
	PBYTE mem = static_cast<PBYTE>(VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	std::cout << "Running tests on " << tests.size() << " different implementations" << std::endl;
	memset(mem, 0, size);
	DWORD dwOld = 0;
	VirtualProtect(mem, si.dwPageSize - 1, PAGE_NOACCESS, &dwOld);
	VirtualProtect(mem + size - si.dwPageSize, si.dwPageSize - 1, PAGE_NOACCESS, &dwOld);
	for (auto x : tests) {
		std::cout << "===========" << std::endl << "Running " << x->name() << std::endl;
		if (!x->run( mem + si.dwPageSize, size - (2*si.dwPageSize) )) {
			std::cout << "FAILED" << std::endl;
		}
	}
	VirtualFree(mem, 0, MEM_RELEASE);
	std::cout << "Done." << std::endl;
	return 0;
}
