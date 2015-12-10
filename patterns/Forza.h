#ifndef Forza_h__
#define Forza_h__

struct PatternData
{
	uint32_t	Count;
	uint32_t	Length[16];
	uint32_t	Mask[16];
	uint32_t	Offset[16];
	__m128i		Value[16];
};

void GeneratePatternArray(const char* Signature, const char* Mask, PatternData* Out)
{
	auto len = strlen(Mask);
	auto len2 = 0;

	Out->Count = 0;

	for (auto i = 0; i < len; i++)
	{
		if (Mask[i] != 'x')
			continue;

		auto mask_begin = 0;
		for (auto j = i; j < len; j++)
		{
			if (Mask[j] == '?')
				break;

			mask_begin++;
		}

		len2 = mask_begin;

		if (len2 + i > len)
			len2 = len - i;

		Out->Value[Out->Count]		= _mm_loadu_si128((const __m128i*)(Signature + i));
		Out->Length[Out->Count]		= len2;
		Out->Offset[Out->Count]		= i;
		Out->Count					+= 1;
		i							+= len2;
	}

	for (auto j = 1; j < Out->Count; j++)
		Out->Mask[j - 1] = Out->Offset[j] - (Out->Offset[j - 1] + Out->Length[j - 1]);
}

__forceinline uint8_t* Find(const uint8_t* Data, const uint32_t Length, const char* Signature, const char* Mask, bool ZeroCheck = true)
{
	PatternData d;
	GeneratePatternArray(Signature, Mask, &d);

	const auto ml			= strlen(Mask);
	const auto branches		= Length / 32;
	const auto end			= Data + Length - ml;

	for (auto i = 0; i < branches; i++)
	{
		auto k = _mm256_load_si256((const __m256i*)Data + i);

		if (ZeroCheck && _mm256_test_all_zeros(k, k) == 1)
			continue;

		auto WithinRegion = [](const __m256i Pointer, PatternData* Patterns)
		{
			auto f = _mm_cmpestri(Patterns->Value[0], Patterns->Length[0], _mm256_extractf128_si256(Pointer, 0), 16, _SIDD_CMP_EQUAL_ORDERED);

			if (f == 16)
				f += _mm_cmpestri(Patterns->Value[0], Patterns->Length[0], _mm256_extractf128_si256(Pointer, 1), 16, _SIDD_CMP_EQUAL_ORDERED);

			return f;
		};

		auto ptr = Data + i * 32;

	Search:
		auto f = WithinRegion(k, &d);

		if (f == 32)
			continue;

		ptr += f;

		if (ptr + ml >= end)
			return nullptr;

		auto ContainsAllSubs = [](const uint8_t* Pointer, PatternData* Patterns)
		{
			auto p = Pointer;

			for (auto i = 0; i < Patterns->Count; i++)
			{
				auto k = _mm_cmpestri(Patterns->Value[i], Patterns->Length[i], _mm_loadu_si128((const __m128i*)p), Patterns->Length[i], _SIDD_CMP_EQUAL_EACH);

				if (k != 0)
					return false;

				p += Patterns->Length[i] + Patterns->Mask[i];
			}

			return true;
		};

		if (ptr[ml - 1] == Signature[ml - 1] && ContainsAllSubs(ptr, &d))
			return (uint8_t*)ptr;

		ptr++;

		if (ptr + 16 >= end)
			return nullptr;

		k = _mm256_load_si256((const __m256i*)ptr);
		goto Search;
	}

	return nullptr;
}

struct Forza : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			Pattern_	= "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
			Mask_		= "xxxxxxx????xxx";
			break;
		case Tests::Second:
			Pattern_	= "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
			Mask_		= "xxxxxxxxxxx????xxx";
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return Find(baseAddress, size, Pattern_, Mask_, true);
	}

	virtual const char* name() const
	{
		return "Forza";
	}

	char* Pattern_;
	char* Mask_;
};

REGISTER(Forza);

#endif // Forza_h__
