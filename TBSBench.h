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

		TBS::State<> state;
		TBS::Pattern::DescriptionBuilder<> templateBuilder;
		const char* currPattern;
	};

	template<typename T>
	FORCEINLINE void ApplySearchRange(Pattern::Description& desc, T _start, T _end)
	{
		auto start = (const UByte*)_start;
		auto end = (const UByte*)_end;
		desc.mLastSearchPos = (const UByte*)start;
		desc.mSearchRangeSlicer = Memory::Slice<const UByte*>::Container(start, end, PATTERN_SEARCH_SLICE_SIZE);
		desc.mCurrentSearchRange = desc.mSearchRangeSlicer.begin();
	}

	TBSBench::TBSBench()
		: templateBuilder(state.PatternBuilder()
			.setUID("Pattern")
			.stopOnFirstMatch())
	{}

	void TBSBench::init(Tests test)
	{
		// Flusing state
		state.mSharedDescriptions.clear();
		switch (test)
		{
		case Tests::First:
			currPattern = "?5 4? 45 ? 3? 9a f? ? ? ? ? 45 ?? 21";
			break;
		case Tests::Second:
			currPattern = "? a? aa a? aa ?? aa ?a aa ? aa ? ? ? ? 45 ?8 2?";
			break;
		default:
			break;
		}
	}

	LPVOID TBSBench::runOne(PBYTE baseAddress, DWORD size)
	{
		TBS::Pattern::Description pattern = 
			templateBuilder
			.Clone()
			.setPattern(currPattern)
			.setScanStart(baseAddress)
			.setScanEnd(baseAddress + size)
			.Build();

		while (Pattern::Scan(pattern))
			(void)0;

		return (LPVOID)(U64)pattern.mShared.mResultAccesor;
	}

	const char* TBSBench::name() const
	{
		return "TBS";
	}

	REGISTER(TBSBench);
}
#endif