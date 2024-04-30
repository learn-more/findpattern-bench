#pragma once

#ifndef TBS_CONTAINER_MAX_SIZE
#define TBS_CONTAINER_MAX_SIZE 16
#endif

#ifndef TBS_STRING_MAX_SIZE
#define TBS_STRING_MAX_SIZE 128
#endif

#ifdef TBS_USE_ETL

#include <etl/to_string.h>


#define STL_ETL(stl,etl) etl
#define TBS_STL_INC(x) STL_ETL(<x>, <etl/x.h>)

#else
#define STL_ETL(stl,etl) stl
#define TBS_STL_INC(x) < x >

#include <string.h>
#endif

#include TBS_STL_INC(string)
#include TBS_STL_INC(unordered_map)
#include TBS_STL_INC(unordered_set)
#include TBS_STL_INC(memory)
#include TBS_STL_INC(vector)
#include STL_ETL(<functional>, <etl/delegate.h>)

#ifdef TBS_MT
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#endif

#ifdef TBS_USE_SSE2
#ifndef TBS_IMPL_SSE2
#define TBS_IMPL_SSE2
#endif
#endif

#ifdef TBS_USE_AVX
#ifndef TBS_IMPL_AVX
#define TBS_IMPL_AVX
#endif
#ifndef TBS_IMPL_SSE2
#define TBS_IMPL_SSE2
#endif
#endif

#ifdef TBS_USE_ARCH_WORD_SIMD
#ifndef TBS_IMPL_ARCH_WORD_SIMD
#define TBS_IMPL_ARCH_WORD_SIMD
#endif
#endif

#ifndef TBS_RESULT_TYPE
#define TBS_RESULT_TYPE U64
#endif

#ifdef TBS_IMPL_SSE2
#ifndef TBS_IMPL_ARCH_WORD_SIMD
#define TBS_IMPL_ARCH_WORD_SIMD
#endif
#include <emmintrin.h>
#endif

#ifdef TBS_IMPL_AVX
#ifndef TBS_IMPL_ARCH_WORD_SIMD
#define TBS_IMPL_ARCH_WORD_SIMD
#endif
#include <immintrin.h>
#endif


#if defined(_MSC_VER)
#pragma intrinsic(_BitScanForward)
#define CTZ(x) [x]{unsigned long index = 0; _BitScanForward((unsigned long *)&index, x); return index; }()
#include <intrin.h> // For __cpuid
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h> // For __get_cpuid
#define CTZ(x) __builtin_ctz(x)
#endif

namespace TBS {
	using U64 = unsigned long long;
	using U32 = unsigned int;
	using UByte = unsigned char;
	using UShort = unsigned short;
	using ULong = unsigned long;
	using UPtr = uintptr_t;

#ifdef TBS_USE_ETL
	template<typename T, typename K, U64 CAPACITY = TBS_CONTAINER_MAX_SIZE>
	using UMap = etl::unordered_map<T, K, CAPACITY>;

	template<typename T, U64 CAPACITY = TBS_CONTAINER_MAX_SIZE>
	using USet = etl::unordered_set<T, CAPACITY>;

	template<typename T, U64 CAPACITY = TBS_CONTAINER_MAX_SIZE>
	using Vector = etl::vector<T, CAPACITY>;

	template<typename T>
	using UniquePtr = etl::unique_ptr<T>;

	template<U64 CAPACITY = TBS_STRING_MAX_SIZE>
	using String = etl::string<CAPACITY>;

	template<typename T>
	using Function = etl::delegate<T>;
#else
	template<typename T, typename K, U64 CAPACITY = TBS_CONTAINER_MAX_SIZE>
	using UMap = std::unordered_map<T, K>;

	template<typename T, U64 CAPACITY = TBS_CONTAINER_MAX_SIZE>
	using USet = std::unordered_set<T>;

	template<typename T, U64 CAPACITY = TBS_CONTAINER_MAX_SIZE>
	using Vector = std::vector<T>;

	template<typename T>
	using UniquePtr = std::unique_ptr<T>;

	template<U64 CAPACITY = TBS_STRING_MAX_SIZE>
	using String = std::string;

	template<typename T>
	using Function = std::function<T>;
#endif

	/*
		Assumed to be 0x1000 for simplicity
	*/
	constexpr U64 PAGE_SIZE = 0x1000;

	/*
		10 Pages per Pattern Slice Scan
	*/
	constexpr U64 PATTERN_SEARCH_SLICE_SIZE = PAGE_SIZE * 10;

#ifdef TBS_MT
	namespace Thread {
		class Pool {
		public:
			inline Pool(size_t threads = std::thread::hardware_concurrency()) : mbStopped(false)
			{
				auto workerTask = [this] {
					for (;;)
					{
						std::function<void()> task;
						{
							std::unique_lock<std::mutex> lock(mTasksMtx);
							mWorkersCondVar.wait(lock, [this] { return mbStopped || !mTasks.empty(); });
							if (mbStopped && mTasks.empty())
								return;
							task = std::move(mTasks.front());
							mTasks.pop();
						}
						task();
					}
					};

				for (size_t i = 0; i < threads; ++i)
					mWorkers.emplace_back(workerTask);
			}

			template<class F, class... Args>
			inline void enqueue(F&& f, Args&&... args) {
				{
					std::unique_lock<std::mutex> lock(mTasksMtx);
					mTasks.emplace(std::bind(std::forward<F>(f), std::forward<Args&&>(args)...));
				}
				mWorkersCondVar.notify_one();
			}

			inline ~Pool() {
				{
					std::unique_lock<std::mutex> lock(mTasksMtx);
					mbStopped = true;
				}
				mWorkersCondVar.notify_all();
				for (std::thread& worker : mWorkers)
					worker.join();
			}

		private:
			Vector<std::thread> mWorkers;
			std::queue<std::function<void()>> mTasks;
			std::mutex mTasksMtx;
			std::condition_variable mWorkersCondVar;
			bool mbStopped;
		};
	}
#endif

	static U64 StringLength(const char* s)
	{
		for (const char* i = s; ; i++)
		{
			if (!*i)
				return (U64)(i - s);
		}

		return 0; // Should Never Get Here
	}

	template<typename ValueT, typename AlignementK>
	ValueT NumberAlignToFloor(ValueT num, AlignementK alignment) {
		return num - num % alignment;
	}

	namespace Memory {
		static UByte Bits4FromChar(char hex)
		{
			if ('0' <= hex && hex <= '9') {
				return hex - '0';
			}
			else if ('A' <= hex && hex <= 'F') {
				return hex - 'A' + 10;
			}
			else if ('a' <= hex && hex <= 'f') {
				return hex - 'a' + 10;
			}

			return 0;
		}

		static UByte ByteFromString(const char* byteStr)
		{
			UByte high = Bits4FromChar(byteStr[0]);
			UByte low = Bits4FromChar(byteStr[1]);

			return (high << 4) | low;
		}

		const UByte* SearchFirstOne2One(const UByte* start, const UByte* end, UByte byte)
		{
			for (const UByte* i = start; i != end; ++i) {
				if (*i != byte)
					continue;

				return i;
			}

			return nullptr; // Byte not found
		}

		inline bool CompareWithMaskOne2One(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* compareMask)
		{
			for (size_t i = 0; i < len; i++)
			{
				UByte b1 = chunk1[i] & compareMask[i];
				UByte b2 = chunk2[i] & compareMask[i];

				if (b1 != b2)
					return false;
			}

			return true;
		}

		namespace SIMD {

			namespace Platform {
#ifdef TBS_IMPL_ARCH_WORD_SIMD
				inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* compareMask) {
					// UPtr ==> Platform Word Length
					size_t wordLen = len / sizeof(UPtr); // Calculate length in words

					for (size_t i = 0; i < wordLen; i++) {
						// Convert byte index to word index
						const UPtr wordMask = *((const UPtr*)compareMask + i);
						const UPtr maskedChunk1 = *((const UPtr*)chunk1 + i) & wordMask;
						const UPtr maskedChunk2 = *((const UPtr*)chunk2 + i) & wordMask;
						if (maskedChunk1 != maskedChunk2)
							return false;
					}

					size_t remainingBytes = len % sizeof(UPtr);

					if (remainingBytes == 0)
						return true;

					// Calculate the starting index of the last incomplete word
					size_t lastWordIndex = wordLen * sizeof(UPtr);
					return Memory::CompareWithMaskOne2One(
						chunk1 + lastWordIndex, chunk2 + lastWordIndex, len - lastWordIndex, compareMask + lastWordIndex);
				}
#endif
			}

#ifdef TBS_IMPL_SSE2
			namespace SSE2 {
				inline bool Supported()
				{
#if defined(_MSC_VER)
					int CPUInfo[4];
					__cpuid(CPUInfo, 1);
					return (CPUInfo[3] & (1 << 26)) != 0; // Check bit 26 of EDX
#elif defined(__GNUC__) || defined(__clang__)
					unsigned int eax, ebx, ecx, edx;
					__get_cpuid(1, &eax, &ebx, &ecx, &edx);
					return (edx & (1 << 26)) != 0; // Check bit 26 of EDX
#else
					// Unsupported compiler/platform
					return false;
#endif
				}

				inline int FirstMatchingByteIndex(__m128i a, __m128i b) {
					__m128i cmp_result = _mm_cmpeq_epi8(a, b);

					// Convert comparison result to mask
					int mask = _mm_movemask_epi8(cmp_result);

					if (mask == 0)
						return -1;

					return CTZ(mask);
				}

				inline const UByte* SearchFirst(const UByte* start, const UByte* end, UByte byte)
				{
					const size_t searchLen = (size_t)(end - start);
					const size_t wordLen = searchLen / sizeof(__m128i); // Calculate length in words

					__m128i searching = _mm_set1_epi8(byte);

					for (size_t i = 0; i < wordLen; i++) {
						const __m128i currScanning = _mm_load_si128((const __m128i*) start + i);

						int foundIndex = FirstMatchingByteIndex(currScanning, searching);

						if (foundIndex < 0)
							continue;

						return (UByte*)((const __m128i*)start + i) + foundIndex;
					}

					if (searchLen % sizeof(__m128i) == 0)
						return nullptr;

					return SearchFirstOne2One(start + wordLen * sizeof(__m128i), end, byte); // Nothing Matched
				}

				inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* compareMask) {
					const size_t wordLen = len / sizeof(__m128i); // Calculate length in words

					for (size_t i = 0; i < wordLen; i++) {
						// Convert byte index to word index
						const __m128i wordMask = _mm_load_si128((const __m128i*) compareMask + i);
						const __m128i maskedChunk1 = _mm_and_si128(_mm_load_si128((const __m128i*)chunk1 + i), wordMask);
						const __m128i maskedChunk2 = _mm_and_si128(_mm_load_si128((const __m128i*)chunk2 + i), wordMask);

						if (_mm_movemask_epi8(_mm_cmpeq_epi32(maskedChunk1, maskedChunk2)) != 0xFFFF)
							return false;
					}

					size_t remainingBytes = len % sizeof(__m128i);

					if (remainingBytes == 0)
						return true;

					const size_t lastWordIndex = wordLen * sizeof(__m128i);

					return Platform::CompareWithMask(chunk1 + lastWordIndex, chunk2 + lastWordIndex, len % sizeof(__m128i), compareMask + lastWordIndex);
				}
			}
#endif


#ifdef TBS_IMPL_AVX
			namespace AVX
			{
				inline bool Supported()
				{
#if defined(_MSC_VER)
					int CPUInfo[4];
					__cpuid(CPUInfo, 1);
					return (CPUInfo[2] & (1 << 28)) != 0; // Check bit 28 of ECX
#elif defined(__GNUC__) || defined(__clang__)
					unsigned int eax, ebx, ecx, edx;
					__get_cpuid(1, &eax, &ebx, &ecx, &edx);
					return (ecx & (1 << 28)) != 0; // Check bit 28 of ECX
#else
					// Unsupported compiler/platform
					return false;
#endif
				}

				inline int FirstMatchingByteIndex(__m256i a, __m256i b) {
					__m256i cmp_result = _mm256_cmpeq_epi8(a, b);

					// Convert comparison result to mask
					int mask = _mm256_movemask_epi8(cmp_result);

					if (mask == 0)
						return -1;

					return CTZ(mask);
				}

				inline const UByte* SearchFirst(const UByte* start, const UByte* end, UByte byte)
				{
					const size_t searchLen = (size_t)(end - start);
					const size_t wordLen = searchLen / sizeof(__m256i); // Calculate length in words

					__m256i searching = _mm256_set1_epi8(byte);

					for (size_t i = 0; i < wordLen; i++) {
						const __m256i currScanning = _mm256_load_si256((const __m256i*) start + i);

						int foundIndex = FirstMatchingByteIndex(currScanning, searching);

						if (foundIndex < 0)
							continue;

						return (UByte*)((const __m256i*)start + i) + foundIndex;
					}

					if (searchLen % sizeof(__m256i) == 0)
						return nullptr;

					return SSE2::SearchFirst(start + wordLen * sizeof(__m256i), end, byte); // Nothing Matched
				}

				inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* compareMask)
				{
					const size_t wordLen = len / sizeof(__m256i); // Calculate length in words

					for (size_t i = 0; i < wordLen; i++)
					{
						// Convert byte index to word index
						const __m256i wordMask = _mm256_load_si256((const __m256i*) compareMask + i);
						const __m256i maskedChunk1 = _mm256_and_si256(_mm256_load_si256((const __m256i*) chunk1 + i), wordMask);
						const __m256i maskedChunk2 = _mm256_and_si256(_mm256_load_si256((const __m256i*) chunk2 + i), wordMask);

						if (_mm256_movemask_epi8(_mm256_cmpeq_epi64(maskedChunk1, maskedChunk2)) != 0xFFFFFFFF)
							return false;
					}

					size_t remainingBytes = len % sizeof(__m256i);

					if (remainingBytes == 0)
						return true;

					const size_t lastWordIndex = wordLen * sizeof(__m256i);

					return SSE2::CompareWithMask(chunk1 + lastWordIndex, chunk2 + lastWordIndex, len % sizeof(__m256i),
						compareMask + lastWordIndex);
				}
			} // namespace AVX
#endif
		}

		inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* compareMask)
		{
			static auto RTCompareWithMask = [] {
#ifdef TBS_USE_AVX
				if (SIMD::AVX::Supported())
					return SIMD::AVX::CompareWithMask;
#elif TBS_USE_SSE2
				if (SIMD::AVX::Supported())
					return SIMD::AVX::CompareWithMask;
#elif defined(TBS_USE_ARCH_WORD_SIMD)
				return SIMD::Platform::CompareWithMask;
#else 
				return CompareWithMaskOne2One;
#endif
				}();

				return RTCompareWithMask(chunk1, chunk2, len, compareMask);
		}

		inline const UByte* SearchFirst(const UByte* start, const UByte* end, UByte byte)
		{
			if (start >= end)
				return nullptr;

			static auto RTSearchFirst = [] {
#ifdef TBS_USE_AVX
				if (SIMD::AVX::Supported())
					return SIMD::AVX::SearchFirst;
#elif TBS_USE_SSE2
				if (SIMD::SSE2::Supported())
					return SIMD::SSE2::SearchFirst;
#else
				return SearchFirstOne2One;
#endif
				}();

				return RTSearchFirst(start, end, byte);
		}

		template<typename T = UPtr>
		struct Slice {
			using PtrTypeT = T;
			using SliceT = Slice<T>;

			inline Slice(PtrTypeT start, PtrTypeT end)
				: mStart(start)
				, mEnd(end)
			{}

			inline bool operator==(const SliceT& other) const
			{
				return
					mStart == other.mStart &&
					mEnd == other.mEnd;
			}

			inline bool operator!=(const SliceT& other) const
			{
				return !(*this == other);
			}

			PtrTypeT mStart;
			PtrTypeT mEnd;

			struct Container {
				inline Container(PtrTypeT start, PtrTypeT end, U64 step)
					: mStart(start)
					, mEnd(end < start ? start : end)
					, mStep(step)
				{}

				struct Iterator {
					inline Iterator(PtrTypeT start, U64 step, PtrTypeT end)
						: mSlice(SliceT(start, (PtrTypeT)((UByte*)start + step)))
						, mStep(step)
						, mEnd(end)
					{
						Normalize();
					}

					inline SliceT operator*() const
					{
						return mSlice;
					}

					inline void Normalize()
					{
						if (mSlice.mStart > mEnd)
							mSlice.mStart = mEnd;

						if (mSlice.mEnd > mEnd)
							mSlice.mEnd = mEnd;
					}

					inline Iterator& operator++()
					{
						mSlice.mStart = (PtrTypeT)((UByte*)mSlice.mStart + mStep);
						mSlice.mEnd = (PtrTypeT)((UByte*)mSlice.mEnd + mStep);

						Normalize();

						return *this;
					}

					inline bool operator==(const Iterator& other) const {
						return mSlice == other.mSlice;
					}

					inline bool operator!=(const Iterator& other) const {
						return !(*this == other);
					}

					SliceT mSlice;
					PtrTypeT mEnd;
					U64 mStep;
				};

				inline Iterator begin() const {
					return Iterator(mStart, mStep, mEnd);
				}

				inline Iterator end() const {
					return Iterator(mEnd, mStep, mEnd);
				}

				PtrTypeT mStart;
				PtrTypeT mEnd;
				U64 mStep;
			};
		};
	}

	namespace Pattern
	{
		using Result = TBS_RESULT_TYPE;
		using Results = Vector<Result>;

		struct ParseResult {
			inline ParseResult()
				: mParseSuccess(false)
				, mFirstSolidOff(-1)
			{}

			inline operator bool()
			{
				return mParseSuccess;
			}

			Vector<UByte> mPattern;
			Vector<UByte> mCompareMask;
			bool mParseSuccess;
			size_t mFirstSolidOff;
		};

		static bool Parse(const void* _pattern, const char* mask, ParseResult& result)
		{
			result = ParseResult();

			if (_pattern == nullptr || mask == nullptr)
				return false;

			size_t patternLen = StringLength(mask);

			result.mPattern.resize(patternLen);
			result.mCompareMask.resize(patternLen);

			memcpy(result.mPattern.data(), _pattern, patternLen);
			memset(result.mCompareMask.data(), 0xFF, patternLen);

			bool bFirstSolidFound = false;

			for (size_t i = 0; i < patternLen; i++)
			{
				if (mask[i] == '?')
				{
					result.mPattern[i] = 0x00;
					result.mCompareMask[i] = 0x00;

					continue;
				}

				if (result.mFirstSolidOff != -1)
					continue;

				result.mFirstSolidOff = i;
			}

			if (result.mFirstSolidOff == -1)
				result.mFirstSolidOff = 0;

			return result.mParseSuccess = true;
		}

		static bool Parse(const String<>& pattern, ParseResult& result)
		{
			result = ParseResult();

			if (pattern.empty())
				return true;

			const char* c = pattern.c_str();
			bool bFirstSolidFound = false;

			for (int i = 0; *c; i++)
			{
				String<> str;

				for (; *c && *c == ' '; c++)
					;

				for (; *c && *c != ' '; c++)
					str.push_back(*c);

				if (str.size() > 2)
					return false;

				// At this point, current pattern byte structure good so far

				bool bAnyWildCard = str.find("?") != String<>::npos;
				bool bFullByteWildcard = str.find("??") != String<>::npos || (bAnyWildCard && str.size() == 1);

				if (!bFullByteWildcard && result.mFirstSolidOff == -1)
					result.mFirstSolidOff = i;

				// At this point, current Byte is wildcarded

				if (bFullByteWildcard)
				{
					// At this point we are dealing with Full Byte Wildcard Case
					result.mPattern.emplace_back(UByte(0x00u));
					result.mCompareMask.emplace_back(UByte(0x00u));
					continue;
				}

				if (!bAnyWildCard)
				{
					// Not Wilcarding this byte
					result.mPattern.emplace_back(Memory::ByteFromString(str.c_str()));
					result.mCompareMask.emplace_back(UByte(0xFFull));
					continue;
				}

				// At this point we are dealing with Half Byte Wildcard Case, "X?" or "?X"

				if (str[0] == '?')
				{
					// At this point, we are dealing with High Part of Byte wildcarding "?X"
					str[0] = '0';
					result.mPattern.emplace_back(Memory::ByteFromString(str.c_str()));
					result.mCompareMask.emplace_back(UByte(0x0Fu));
					continue;
				}

				// At this point, we are dealing with Low Part of Byte wildcarding "X?"
				str[1] = '0';
				result.mPattern.emplace_back(Memory::ByteFromString(str.c_str()));
				result.mCompareMask.emplace_back(UByte(0xF0u));
			}

			if (result.mFirstSolidOff == -1)
				result.mFirstSolidOff = 0;

			return result.mParseSuccess = true;
		}

		static bool Valid(const String<>& pattern)
		{
			ParseResult res;

			return Parse(pattern, res) && res;
		}

		static bool Valid(const void* _pattern, const char* mask)
		{
			ParseResult res;

			return Parse(_pattern, mask, res) && res;
		}

		enum class EScan {
			SCAN_ALL,
			SCAN_FIRST
		};

		struct Description {
			using ResultTransformer = Function<Result(Description&, Result)>;
			using SearchSlice = Memory::Slice<const UByte*>;

			struct Shared {
				struct ResultAccesor {
					inline ResultAccesor(Shared& sharedDesc)
						: mSharedDesc(sharedDesc)
					{}

					inline operator const Results& () const {
						return ResultsGet();
					}

					inline operator Result () const {
						if (mSharedDesc.mResult.size() < 1)
							return 0;

						return mSharedDesc.mResult[0];
					}

					inline const Results& ResultsGet() const
					{
						return mSharedDesc.mResult;
					}

					Shared& mSharedDesc;
				};

				inline Shared(EScan scanType)
					: mScanType(scanType)
					, mResultAccesor(*this)
					, mFinished(false)
				{}

#ifdef TBS_MT
				std::mutex mMutex;
				std::atomic<bool> mFinished;
#else
				bool mFinished;
#endif
				EScan mScanType;
				Results mResult;
				ResultAccesor mResultAccesor;
			};

			inline Description(Shared& shared, const String<>& uid, const UByte* searchStart, const UByte* searchEnd,
				const Vector<ResultTransformer>& transformers, const String<>& pattern)
				: Description(shared, uid, searchStart, searchEnd, transformers)
			{
				Parse(pattern, mParsed);
			}

			inline Description(
				Shared& shared, const String<>& uid,
				const UByte* searchStart, const UByte* searchEnd,
				const Vector<ResultTransformer>& transformers, const void* _pattern, const char* mask)
				: Description(shared, uid, searchStart, searchEnd, transformers)
			{
				Parse(_pattern, mask, mParsed);
			}

			inline operator bool()
			{
				return mParsed;
			}

			Shared& mShared;
			String<> mUID;
			Vector<ResultTransformer> mTransforms;
			SearchSlice::Container mSearchRangeSlicer;
			SearchSlice::Container::Iterator mCurrentSearchRange;
			const UByte* mLastSearchPos;
			ParseResult mParsed;

		private:

			inline Description(Shared& shared, const String<>& uid, const UByte* searchStart, const UByte* searchEnd,
				const Vector<ResultTransformer>& transformers)
				: mShared(shared)
				, mUID(uid)
				, mTransforms(transformers)
				, mSearchRangeSlicer(searchStart, searchEnd, PATTERN_SEARCH_SLICE_SIZE)
				, mCurrentSearchRange(mSearchRangeSlicer.begin())
				, mLastSearchPos(searchStart)
			{ }
		};

		using ResultTransformer = Description::ResultTransformer;

		static bool Scan(Description& desc)
		{
			if (desc.mShared.mFinished ||
				(desc.mCurrentSearchRange == desc.mSearchRangeSlicer.end()))
				return false;

			const size_t patternLen = desc.mParsed.mPattern.size();

			Description::SearchSlice currSearchnigRange = (*desc.mCurrentSearchRange);

			const auto firstWildcard = desc.mParsed.mCompareMask.at(desc.mParsed.mFirstSolidOff);
			const auto firstPatternByte = desc.mParsed.mPattern.at(desc.mParsed.mFirstSolidOff) & firstWildcard;
			const auto patternSize = desc.mParsed.mPattern.size();

			for (
				const UByte* found = Memory::SearchFirst(desc.mLastSearchPos, currSearchnigRange.mEnd, firstPatternByte);
				found && (found + patternSize - 1) < currSearchnigRange.mEnd;
				found = Memory::SearchFirst(found + 1, currSearchnigRange.mEnd, firstPatternByte))
			{
				if (desc.mShared.mFinished)
					return false;

				auto scanEntry = found - desc.mParsed.mFirstSolidOff;

				if (Memory::CompareWithMask(scanEntry, desc.mParsed.mPattern.data(), desc.mParsed.mPattern.size(),
					desc.mParsed.mCompareMask.data()
				) == false)
					continue;

				Result currMatch = (Result)scanEntry;

				// At this point, we found a match

				for (const auto& transform : desc.mTransforms)
					currMatch = transform(desc, currMatch);

				// At this point, match is properly user transformed
				// lets report it
				{
#ifdef TBS_MT
					std::lock_guard<std::mutex> resultReportLck(desc.mShared.mMutex);
#endif

					if (desc.mShared.mFinished)
						break;

					// At this point, we have the lock & we havent finished!
					// Lets directly push it

					desc.mShared.mResult.push_back(currMatch);

					if (desc.mShared.mScanType != EScan::SCAN_FIRST)
						continue;

					// At this point seems we are searching for a single result
					// lets report finished state for the shared state & break 
					// current search.

					desc.mShared.mFinished = true;
					return false;
				}
			}

			desc.mLastSearchPos = currSearchnigRange.mEnd - patternSize;
			++desc.mCurrentSearchRange;
			return !(desc.mCurrentSearchRange == desc.mSearchRangeSlicer.end());
		}

		using SharedDescription = Description::Shared;
		using SharedResultAccesor = SharedDescription::ResultAccesor;
	}

	namespace Pattern {
		template<U32 SHAREDDESCS_CAPACITY = TBS_CONTAINER_MAX_SIZE>
		struct DescriptionBuilder {

			inline DescriptionBuilder(UMap<String<>, UniquePtr<Pattern::SharedDescription>, SHAREDDESCS_CAPACITY>& sharedDescriptions)
				: mSharedDescriptions(sharedDescriptions)
				, mScanStart(0)
				, mScanEnd(0)
				, mScanType(EScan::SCAN_ALL)
				, mRawPattern(0)
				, mRawMask(0)
			{}

			inline DescriptionBuilder& setPattern(const String<>& pattern)
			{
				mPattern = pattern;

				if (!mUID.empty())
					return *this;

				return setUID(pattern);
			}

			inline DescriptionBuilder& setPatternRaw(const void* pattern)
			{
				mRawPattern = pattern;

				if (!mUID.empty())
					return *this;

#ifdef TBS_USE_ETL
				TBS::String<> uid;
				uid = etl::to_string((UPtr)pattern, uid);
				return setUID(uid);
#else
				return setUID(std::to_string((UPtr)pattern));
#endif
			}

			inline DescriptionBuilder& setMask(const char* mask)
			{
				mRawMask = mask;
				return *this;
			}

			inline DescriptionBuilder& setUID(const String<>& uid)
			{
				mUID = uid;
				return *this;
			}

			template<typename T>
			inline DescriptionBuilder& setScanStart(T start)
			{
				mScanStart = (const UByte*)start;
				return *this;
			}

			template<typename T>
			inline DescriptionBuilder& setScanEnd(T end, bool bOffsetFromStart = false)
			{
				mScanEnd = (const UByte*)end;

				if (bOffsetFromStart)
					mScanEnd += (UPtr)mScanStart;

				return *this;
			}

			inline DescriptionBuilder& AddTransformer(const Description::ResultTransformer& transformer)
			{
				mTransformers.emplace_back(transformer);
				return *this;
			}

			inline DescriptionBuilder& setScanType(EScan type)
			{
				mScanType = type;
				return *this;
			}

			inline DescriptionBuilder& stopOnFirstMatch()
			{
				return setScanType(EScan::SCAN_FIRST);
			}

			inline DescriptionBuilder Clone() const
			{
				return DescriptionBuilder(*this);
			}

			inline Description Build()
			{
				if (!(mRawPattern && mRawMask) && Pattern::Valid(mPattern) == false)
				{
					static SharedDescription nullSharedDesc(EScan::SCAN_ALL);
					static Description nullDescription(nullSharedDesc, "", 0, 0, {}, "");
					return nullDescription;
				}

				if (mSharedDescriptions.find(mUID) == mSharedDescriptions.end())
					mSharedDescriptions[mUID] = UniquePtr<SharedDescription>(new SharedDescription(mScanType));

				if (mRawPattern && mRawMask)
					return Description(*mSharedDescriptions[mUID], mUID, mScanStart, mScanEnd, mTransformers, mRawPattern, mRawMask);

				return Description(*mSharedDescriptions[mUID], mUID, mScanStart, mScanEnd, mTransformers, mPattern);
			}

		private:
			UMap<String<>, UniquePtr<Pattern::SharedDescription>, SHAREDDESCS_CAPACITY>& mSharedDescriptions;
			EScan mScanType;
			String<> mPattern;
			const void* mRawPattern;
			const char* mRawMask;
			String<> mUID;
			const UByte* mScanStart;
			const UByte* mScanEnd;
			Vector<ResultTransformer> mTransformers;
		};
	}

	template<U64 SHAREDDESCS_CAPACITY = TBS_CONTAINER_MAX_SIZE, U64 DESCS_CAPACITY = SHAREDDESCS_CAPACITY * 2>
	struct State {

		using DescriptionBuilderT = Pattern::DescriptionBuilder<SHAREDDESCS_CAPACITY>;

		inline State()
			: State(nullptr, nullptr)
		{}

		template<typename T, typename K>
		inline State(T defScanStart = (T)0, K defScanEnd = (K)0)
			: mDefaultScanStart((const UByte*)defScanStart)
			, mDefaultScanEnd((const UByte*)defScanEnd)
		{}

		inline State& AddPattern(Pattern::Description&& pattern)
		{
			mDescriptionts.emplace_back(pattern);
			return *this;
		}

		inline DescriptionBuilderT PatternBuilder()
		{
			return DescriptionBuilderT(mSharedDescriptions)
				.setScanStart(mDefaultScanStart)
				.setScanEnd(mDefaultScanEnd);
		}

		inline Pattern::SharedResultAccesor operator[](const String<>& uid) const
		{
			if (mSharedDescriptions.find(uid) != mSharedDescriptions.end())
				return *(mSharedDescriptions.at(uid));

			static Pattern::SharedDescription nullSharedDesc(Pattern::EScan::SCAN_ALL);

			return nullSharedDesc;
		}

		const UByte* mDefaultScanStart;
		const UByte* mDefaultScanEnd;
		UMap<String<>, UniquePtr<Pattern::SharedDescription>, SHAREDDESCS_CAPACITY> mSharedDescriptions;
		Vector<Pattern::Description, DESCS_CAPACITY> mDescriptionts;
	};

	template<typename StateT>
	static bool Scan(StateT& state)
	{
		USet<Pattern::Description*> doneSearchingDescs;

#ifdef TBS_MT
		std::mutex doneSearchingDescsMtx;
#endif

		while (doneSearchingDescs.size() < state.mDescriptionts.size())
		{
#ifdef TBS_MT
			Thread::Pool threadPool;
#endif
			for (Pattern::Description& _description : state.mDescriptionts)
			{
				Pattern::Description* description = &_description;

				{
#ifdef TBS_MT
					std::lock_guard<std::mutex> lck(doneSearchingDescsMtx);
#endif

					if (doneSearchingDescs.find(description) != doneSearchingDescs.end())
						continue;
				}
#ifdef TBS_MT
				threadPool.enqueue(
					[&doneSearchingDescs, &doneSearchingDescsMtx](Pattern::Description* description)
					{
#endif
						if (Pattern::Scan(*description) == false)
						{
#ifdef TBS_MT
							std::lock_guard<std::mutex> lck(doneSearchingDescsMtx);
#endif
							doneSearchingDescs.insert(description);
						}

#ifdef TBS_MT
					}, description);
#endif
			}
		}

		state.mDescriptionts.clear();

		bool bAllFoundAny = true;

		for (auto& sharedDescKv : state.mSharedDescriptions)
			bAllFoundAny = bAllFoundAny && sharedDescKv.second->mResult.empty() == false;

		return bAllFoundAny;
	}

	template<typename K, U64 SHAREDDESCS_CAPACITY = TBS_CONTAINER_MAX_SIZE, U64 DESCS_CAPACITY = SHAREDDESCS_CAPACITY * 2>
	static bool ScanOne(K start, K end, const String<>& pattern, Pattern::Result& outResult)
	{
		outResult = Pattern::Result{};

		State<SHAREDDESCS_CAPACITY, DESCS_CAPACITY> state(start, end);

		state.AddPattern(
			state.PatternBuilder()
			.setPattern(pattern)
			.stopOnFirstMatch()
			.Build()
		);

		if (!Scan(state))
			return false;

		outResult = state[pattern];

		return true;
	}

	namespace Light {
		template<typename T>
		inline bool Scan(T _start, T _end, Pattern::Results& results, const void* pattern, const char* mask)
		{
			Pattern::ParseResult parse;

			if (Pattern::Parse(pattern, mask, parse) == false)
				return false;


			return Scan<T>(_start, _end, results, parse);
		}

		template<typename T>
		inline bool Scan(T _start, T _end, Pattern::Results& results, const char* pattern)
		{
			Pattern::ParseResult parse;

			if (Pattern::Parse(pattern, parse) == false)
				return false;


			return Scan<T>(_start, _end, results, parse);
		}

		template<typename T>
		inline bool Scan(T _start, T _end, Pattern::Results& results, const Pattern::ParseResult& parse)
		{
			results.clear();

			const UByte* start = (decltype(start))_start;
			const UByte* end = (decltype(end))_end;

			const auto firstWildcard = parse.mCompareMask.at(parse.mFirstSolidOff);
			const auto firstPatternByte = parse.mPattern.at(parse.mFirstSolidOff) & firstWildcard;

			for (
				const UByte* found = Memory::SearchFirst(start, end, firstPatternByte);
				found && (found + parse.mPattern.size() - 1) < end;
				found = Memory::SearchFirst(found + 1, end, firstPatternByte))
			{
				if (!Memory::CompareWithMask(
					found - parse.mFirstSolidOff, parse.mPattern.data(), parse.mPattern.size(), parse.mCompareMask.data()))
					continue;

				results.push_back((TBS_RESULT_TYPE)(found - parse.mFirstSolidOff));
			}

			return results.empty() == false;
		}

		template<typename T>
		inline bool ScanOne(T _start, T _end, Pattern::Result& result, const void* pattern, const char* mask)
		{
			Pattern::ParseResult parse;

			if (Pattern::Parse(pattern, mask, parse) == false)
				return false;


			return ScanOne<T>(_start, _end, result, parse);
		}

		template<typename T>
		inline bool ScanOne(T _start, T _end, Pattern::Result& result, const char* pattern)
		{
			Pattern::ParseResult parse;

			if (Pattern::Parse(pattern, parse) == false)
				return false;


			return ScanOne<T>(_start, _end, result, parse);
		}

		template<typename T>
		inline bool ScanOne(T _start, T _end, Pattern::Result& result, const Pattern::ParseResult& parse)
		{
			const UByte* start = (decltype(start))_start;
			const UByte* end = (decltype(end))_end;

			const auto firstWildcard = parse.mCompareMask.at(parse.mFirstSolidOff);
			const auto firstPatternByte = parse.mPattern.at(parse.mFirstSolidOff) & firstWildcard;

			for (
				const UByte* found = Memory::SearchFirst(start, end, firstPatternByte);
				found && (found + parse.mPattern.size() - 1) < end;
				found = Memory::SearchFirst(found + 1, end, firstPatternByte))
			{
				if (!Memory::CompareWithMask(
					found - parse.mFirstSolidOff, parse.mPattern.data(), parse.mPattern.size(), parse.mCompareMask.data()))
					continue;

				result = (TBS_RESULT_TYPE)(found - parse.mFirstSolidOff);
				return true;
			}

			return false;
		}
	}
}