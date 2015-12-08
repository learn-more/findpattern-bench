#ifndef FORZA_H
#define FORZA_H

struct PatternData
{
    uint32_t		Count;
    __m128i		Data[32];
};

void GeneratePatternArray( const char* Signature, const char* Mask, PatternData* Out )
{
    auto len = strlen( Mask );
    auto count = len;

    if (count < 16 || count % 16)
        count = (count / 16) + 1;
    else
        count /= 16;

    Out->Count = count;

    for (auto i = 0; i < count; i++)
        Out->Data[i] = _mm_loadu_si128( (const __m128i*)Signature + i );
}

uint8_t* SubScan( const uint8_t* Data, uint32_t Max, PatternData* Patterns, const char* Signature, const char* Mask )
{
    const auto s = strlen( Mask );

    for (auto i = 0; i < Patterns->Count; i++)
    {
        if (Data[s - 1] != Signature[s - 1])
            return nullptr;

        auto k = _mm_loadu_si128( (const __m128i*)Data + i );

        for (auto j = 0; j < s; j++)
        {
            if (Mask[(i * 16) + j] != 'x')
                Patterns->Data[i].m128i_u8[j] = k.m128i_u8[j];
        }

        if (i + 1 >= Patterns->Count)
        {
            auto cutoff = ((i + 1) * 16) - s;

            for (auto j = 0; j <= cutoff; j++)
                Patterns->Data[i].m128i_u8[16 - j] = k.m128i_u8[16 - j];
        }

        auto cmp = _mm_xor_si128( k, Patterns->Data[i] );

        if (_mm_test_all_zeros( cmp, cmp ) != 1)
            return nullptr;

        if (i + 1 >= Patterns->Count)
            return (uint8_t*)Data;
    }

    return nullptr;
}

uint8_t* Find( const uint8_t* Data, const uint32_t Length, const char* Signature, const char* Mask )
{
    PatternData d;
    GeneratePatternArray( Signature, Mask, &d );

    const auto nl = strlen( Signature );
    const auto branches = Length / 32;
    const auto first = d.Data[0];
    const auto end = Data + Length - nl;

    for (auto i = 0; i < branches - 1; i++)
    {
        auto k = _mm256_load_si256( (const __m256i*)Data + i );

        if (_mm256_test_all_zeros( k, k ) == 1)
            continue;

        auto f = _mm_cmpestri( first, nl, _mm_loadu_si128( (const __m128i*)(Data + i * 32) ), 16, _SIDD_CMP_EQUAL_ORDERED );

        if (f == 16)
        {
            f = _mm_cmpestri( first, nl, _mm_loadu_si128( (const __m128i*)(Data + i * 32 + 16) ), 16, _SIDD_CMP_EQUAL_ORDERED );

            if (f == 16)
                continue;

            f += 16;
        }

        uint8_t* p = (uint8_t*)(Data + (i * 32) + f);
        p = SubScan( p, static_cast<uint32_t>(end - p), &d, Signature, Mask );

        if (p != nullptr)
            return p;
    }

    return nullptr;
}

struct FORZA : public BenchBase
{
    virtual void init( Tests test )
    {
        switch (test)
        {
        case Tests::First:
            pattern = "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
            mask = "xxxxxxx????xxx";
            break;
        case Tests::Second:
            pattern = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
            mask = "xxxxxxxxxxx????xxx";
            break;
        }
    }

    virtual LPVOID runOne( PBYTE baseAddress, DWORD size )
    {
        return Find( baseAddress, size, pattern, mask );
    }
    virtual const char* name() const
    {
        return "Forza";
    }

    char* pattern;
    char* mask;
};

REGISTER( FORZA );

#endif // FORZA_H
