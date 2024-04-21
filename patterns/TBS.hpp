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
#include <emmintrin.h>
#endif

#ifdef TBS_IMPL_AVX
#include <immintrin.h>
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

		inline bool CompareWithMaskByte(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* wildCardMask)
		{
			if ((chunk1[0] & ~wildCardMask[0]) != (chunk2[0] & ~wildCardMask[0]))
				return false;

			for (size_t i = 0; i < len; i++)
			{
				UByte b1 = chunk1[i] & ~wildCardMask[i];
				UByte b2 = chunk2[i] & ~wildCardMask[i];

				if (b1 != b2)
					return false;
			}

			return true;
		}

#ifdef TBS_IMPL_ARCH_WORD_SIMD
		inline bool CompareWithMaskWord(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* wildCardMask) {
			if ((chunk1[0] & ~wildCardMask[0]) != (chunk2[0] & ~wildCardMask[0]))
				return false;

			// UPtr ==> Platform Word Length
			size_t wordLen = len / sizeof(UPtr); // Calculate length in words

			for (size_t i = 0; i < wordLen; i++) {
				// Convert byte index to word index
				UPtr wordMask = ~*((const UPtr*)wildCardMask + i);
				UPtr maskedChunk1 = *((const UPtr*)chunk1 + i) & wordMask;
				UPtr maskedChunk2 = *((const UPtr*)chunk2 + i) & wordMask;
				if (maskedChunk1 != maskedChunk2)
					return false;
			}

			size_t remainingBytes = len % sizeof(UPtr);

			if (remainingBytes == 0)
				return true;

			// Calculate the starting index of the last incomplete word
			size_t lastWordIndex = wordLen * sizeof(UPtr);
			return CompareWithMaskByte(
				chunk1 + lastWordIndex, chunk2 + lastWordIndex, len - lastWordIndex, wildCardMask + lastWordIndex);
		}
#endif

#ifdef TBS_IMPL_SSE2
		namespace SSE2 {
			inline __m128i _mm_not_si128(__m128i value)
			{
				__m128i mask = _mm_set1_epi32(-1);
				return _mm_xor_si128(value, mask);
			}

			inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* wildCardMask) {

				if ((chunk1[0] & ~wildCardMask[0]) != (chunk2[0] & ~wildCardMask[0]))
					return false;

				const size_t wordLen = len / sizeof(__m128i); // Calculate length in words

				for (size_t i = 0; i < wordLen; i++) {
					// Convert byte index to word index
					__m128i wordMask = _mm_not_si128(_mm_load_si128((const __m128i*)wildCardMask + i));
					__m128i maskedChunk1 = _mm_and_si128(_mm_load_si128((const __m128i*)chunk1 + i), wordMask);
					__m128i maskedChunk2 = _mm_and_si128(_mm_load_si128((const __m128i*)chunk2 + i), wordMask);

					if (_mm_movemask_epi8(_mm_cmpeq_epi32(maskedChunk1, maskedChunk2)) != 0xFFFF)
						return false;
				}

				size_t remainingBytes = len % sizeof(__m128i);

				if (remainingBytes == 0)
					return true;

				const size_t lastWordIndex = wordLen * sizeof(__m128i);

				return CompareWithMaskWord(chunk1 + lastWordIndex, chunk2 + lastWordIndex, len % sizeof(__m128i), wildCardMask + lastWordIndex);
			}
		}
#endif


#ifdef TBS_IMPL_AVX
		namespace AVX
		{
			inline __m256i _mm256_not_si256(__m256i value)
			{
				__m256i mask = _mm256_set1_epi32(-1);
				return _mm256_xor_si256(value, mask);
			}

			inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* wildCardMask)
			{
				if ((chunk1[0] & ~wildCardMask[0]) != (chunk2[0] & ~wildCardMask[0]))
					return false;

				const size_t wordLen = len / sizeof(__m256i); // Calculate length in words

				for (size_t i = 0; i < wordLen; i++)
				{
					// Convert byte index to word index
					__m256i wordMask = _mm256_not_si256(_mm256_load_si256((const __m256i*) wildCardMask + i));
					__m256i maskedChunk1 = _mm256_and_si256(_mm256_load_si256((const __m256i*) chunk1 + i), wordMask);
					__m256i maskedChunk2 = _mm256_and_si256(_mm256_load_si256((const __m256i*) chunk2 + i), wordMask);

					if (_mm256_movemask_epi8(_mm256_cmpeq_epi64(maskedChunk1, maskedChunk2)) != 0xFFFFFFFF)
						return false;
				}

				size_t remainingBytes = len % sizeof(__m256i);

				if (remainingBytes == 0)
					return true;

				const size_t lastWordIndex = wordLen * sizeof(__m256i);

				return SSE2::CompareWithMask(chunk1 + lastWordIndex, chunk2 + lastWordIndex, len % sizeof(__m256i),
					wildCardMask + lastWordIndex);
			}
		} // namespace AVX
#endif


		inline bool CompareWithMask(const UByte* chunk1, const UByte* chunk2, size_t len, const UByte* wildCardMask)
		{
#ifdef TBS_USE_AVX
			return AVX::CompareWithMask(chunk1, chunk2, len, wildCardMask);
#elif TBS_USE_SSE2
			return SSE2::CompareWithMask(chunk1, chunk2, len, wildCardMask);
#elif defined(TBS_USE_ARCH_WORD_SIMD)
			return CompareWithMaskWord(chunk1, chunk2, len, wildCardMask);
#else 
			return CompareWithMaskByte(chunk1, chunk2, len, wildCardMask);
#endif
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
				, mFirstSolidOff(0)
			{}

			inline operator bool()
			{
				return mParseSuccess;
			}

			Vector<UByte> mPattern;
			Vector<UByte> mWildcardMask;
			bool mParseSuccess;
			U32 mFirstSolidOff;
			U32 mFirstSolidOffAlign;
			U32 mFirstSolidOffAlignPatternLen;

			inline const UByte* getPatternFirstSolidAlignEntry()
			{
				return mPattern.data() + mFirstSolidOffAlign;
			}

			inline const UByte* getMaskFirstSolidAlignEntry()
			{
				return mWildcardMask.data() + mFirstSolidOffAlign;
			}
		};

		static bool Parse(const void* _pattern, const char* mask, ParseResult& result)
		{
			result = ParseResult();

			if (_pattern == nullptr || mask == nullptr)
				return false;

			size_t patternLen = StringLength(mask);

			result.mPattern.resize(patternLen);
			result.mWildcardMask.resize(patternLen);

			memcpy(result.mPattern.data(), _pattern, patternLen);
			memset(result.mWildcardMask.data(), 0x00, patternLen);

			bool bFirstSolidFound = false;

			for (size_t i = 0; i < patternLen; i++)
			{
				if (mask[i] == '?')
				{
					result.mPattern[i] = 0x00;
					result.mWildcardMask[i] = 0xFF;

					continue;
				}

				if (bFirstSolidFound)
					continue;

				result.mFirstSolidOff = i;

				bFirstSolidFound = true;
			}

			result.mFirstSolidOffAlign = NumberAlignToFloor(result.mFirstSolidOff, sizeof(TBS::UPtr));
			result.mFirstSolidOffAlignPatternLen = result.mPattern.size() - result.mFirstSolidOffAlign;

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

				if (!bFirstSolidFound && !bFullByteWildcard)
				{
					result.mFirstSolidOff = i;
					bFirstSolidFound = true;
				}

				// At this point, current Byte is wildcarded

				if (bFullByteWildcard)
				{
					// At this point we are dealing with Full Byte Wildcard Case
					result.mPattern.emplace_back(UByte(0x00u));
					result.mWildcardMask.emplace_back(UByte(0xFFu));
					continue;
				}

				if (!bAnyWildCard)
				{
					// Not Wilcarding this byte
					result.mPattern.emplace_back(Memory::ByteFromString(str.c_str()));
					result.mWildcardMask.emplace_back(UByte(0x00ull));
					continue;
				}

				// At this point we are dealing with Half Byte Wildcard Case, "X?" or "?X"

				if (str[0] == '?')
				{
					// At this point, we are dealing with High Part of Byte wildcarding "?X"
					str[0] = '0';
					result.mPattern.emplace_back(Memory::ByteFromString(str.c_str()));
					result.mWildcardMask.emplace_back(UByte(0xF0u));
					continue;
				}

				// At this point, we are dealing with Low Part of Byte wildcarding "X?"
				str[1] = '0';
				result.mPattern.emplace_back(Memory::ByteFromString(str.c_str()));
				result.mWildcardMask.emplace_back(UByte(0x0Fu));
			}

			result.mFirstSolidOffAlign = NumberAlignToFloor(result.mFirstSolidOff, sizeof(TBS::UPtr));
			result.mFirstSolidOffAlignPatternLen = result.mPattern.size() - result.mFirstSolidOffAlign;

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

			for (const UByte*& i = desc.mLastSearchPos; i + patternLen - 1 < currSearchnigRange.mEnd; i++)
			{
				if (desc.mShared.mFinished)
					return false;

				if (Memory::CompareWithMask(
					i,
					desc.mParsed.getPatternFirstSolidAlignEntry(),
					desc.mParsed.mFirstSolidOffAlignPatternLen,
					desc.mParsed.getMaskFirstSolidAlignEntry()
				) == false)
					continue;

				Result currMatch = (Result)i;

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
}