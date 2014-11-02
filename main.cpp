#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <vector>
#include "Timer.h"

#include <stdint.h>
#include <algorithm>
#include <random>

enum { kNumPages = 22 };
enum { kNumRuns = 1000 };

#include "Pattern.h"
#include "PatternScanner.h"
#include "PatternTestBase.h"

// Register tests
namespace MemoryBounds
{
#include "PatternTestBounds.h"
}
namespace PatternCheating
{
#include "PatternTestCheat.h"
}
namespace ZeroMem
{
#include "PatternTestZeroMem.h"
}
namespace Randomize
{
#include "PatternTestRandomize.h"
}

// Register patterns
REG_PATTERN( 0, "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21", "45 43 45 55 33 9A FA ? ? ? ? 45 68 21", "xxxxxxx????xxx" );
REG_PATTERN( 1, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21", "AA AA AA AA AA AA AA AA AA BB AA ? ? ? ? 45 68 21", "xxxxxxxxxxx????xxx" );
REG_PATTERN( 2, "\x45\x43\x00\x55\x33\x00\xfa\x00\x00\x68\x67\x45\x68\x21\x34\x52", "45 43 ? 55 33 ? FA ? ? 68 67 45 68 21 34 52", "xx?xx?x??xxxxxxx" );
REG_PATTERN( 3, "\x45\x43\x32\x55\x33\xc2\xfa\x68\x67\x45\x68\x21\x34\x52", "45 43 32 55 33 C2 FA 68 67 45 68 21 34 52", "xxxxxxxxxxxxxx" );

// Register scanners
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
namespace afffsdd
{
#include "patterns/afffsdd.h"
}
namespace DarthTon
{
#include "patterns/DarthTon.h"
}
namespace kokole
{
#include "patterns/kokole.h"
}


int main()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	std::cout << "FindPattern benchmark" << std::endl;
	std::cout << "Page size: " << si.dwPageSize << ", allocating " << kNumPages << " pages (including 2 guard pages)." << std::endl;
	DWORD size = si.dwPageSize * kNumPages;
	PBYTE mem = static_cast<PBYTE>(VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	std::cout << "Running tests on " << g_scanners.size() << " different implementations" << std::endl;
	memset(mem, 0, size);
	DWORD dwOld = 0;
	VirtualProtect(mem, si.dwPageSize - 1, PAGE_NOACCESS, &dwOld);
	VirtualProtect(mem + size - si.dwPageSize, si.dwPageSize - 1, PAGE_NOACCESS, &dwOld);
	for (auto x : g_tests) {
        std::cout << std::endl << std::endl << "---------" << x->Name() << "---------" << std::endl;
        x->SetMemoryRegion( mem + si.dwPageSize, size - (2 * si.dwPageSize) );
        x->RunAll( g_scanners, g_patterns );
	}
	VirtualFree(mem, 0, MEM_RELEASE);
	std::cout << "Done." << std::endl;
	return 0;
}
