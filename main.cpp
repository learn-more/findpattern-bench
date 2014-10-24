#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <vector>
#include "Timer.h"

#include <stdint.h>
#include <algorithm>

const char szPattern1[] = "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
const char szWildcard1[] = "xxxxxxx????xxx";

const char szPattern2[] = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
const char szWildcard2[] = "xxxxxxxxxxx????xxx";

enum { kNumPages = 22 };
enum { kNumRuns = 1000 };

#include "BenchBase.h"

std::vector<BenchBase*> tests;

size_t addTest(BenchBase* b)
{
	tests.push_back(b);
	return tests.size();
}

#define XREGISTER2(x,y)	static size_t dummy_reg_##y = addTest( new x() );
#define XREGISTER(x,y)	XREGISTER2(x, y)
#define REGISTER(x)		XREGISTER(x, __LINE__)

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
namespace fdsasdf
{
#include "patterns/fdsasdf.h"
}
namespace DarthTon
{
#include "patterns/DarthTon.h"
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
