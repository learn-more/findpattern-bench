#ifndef TBS_TESTS_INCLUDE
#define TBS_TESTS_INCLUDE

#include "patterns/TBS.hpp"

namespace TBS {
	struct TBSBench : public BenchBase
	{
		TBSBench();

		virtual void init(Tests test);
		virtual LPVOID runOne(PBYTE baseAddress, DWORD size);
		virtual const char* name() const;

		const void* currPattern;
		const char* currMask;
	};

	TBSBench::TBSBench()
	{}

	void TBSBench::init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			currPattern = "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
			currMask = "xxxxxxx????xxx";
			break;
		case Tests::Second:
			currPattern = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
			currMask = "xxxxxxxxxxx????xxx";
			break;
		default:
			break;
		}
	}

	LPVOID TBSBench::runOne(PBYTE baseAddress, DWORD size)
	{
		Pattern::Result r;
		Light::ScanOne(baseAddress, baseAddress + size, r, currPattern, currMask);
		return (LPVOID)r;
	}

	const char* TBSBench::name() const
	{
		return "TBS";
	}

	REGISTER(TBSBench);
}
#endif