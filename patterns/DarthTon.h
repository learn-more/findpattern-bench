#ifndef DARTHTON_H
#define DARTHTON_H

// Boyer-Moore-Horspool with wildcards implementation
void FillShiftTable( const uint8_t* pPattern, size_t patternSize, uint8_t wildcard, size_t* bad_char_skip )
{
    size_t idx = 0;
    size_t last = patternSize - 1;

    // Get last wildcard position
    for (idx = last; idx > 0 && pPattern[idx] != wildcard; --idx);
    size_t diff = last - idx;
    if (diff == 0)
        diff = 1;

    // Prepare shift table
    for (idx = 0; idx <= UCHAR_MAX; ++idx)
        bad_char_skip[idx] = diff;
    for (idx = last - diff; idx < last; ++idx)
        bad_char_skip[pPattern[idx]] = last - idx;
}

const void* Search( const uint8_t* pScanPos, size_t scanSize, const uint8_t* pPattern, size_t patternSize, uint8_t wildcard )
{
    size_t bad_char_skip[UCHAR_MAX + 1];
    const uint8_t* scanEnd = pScanPos + scanSize - patternSize;
    intptr_t last = static_cast<intptr_t>(patternSize) - 1;

    FillShiftTable( pPattern, patternSize, wildcard, bad_char_skip );

    // Search
    for (; pScanPos <= scanEnd; pScanPos += bad_char_skip[pScanPos[last]])
    {
        for (intptr_t idx = last; idx >= 0 ; --idx)
            if (pPattern[idx] != wildcard && pScanPos[idx] != pPattern[idx])
                goto skip;
            else if (idx == 0)
                return pScanPos;
    skip:;
    }

    return nullptr;
}


struct DARTH_TON : public BenchBase
{
    virtual void init( Tests test )
    {
        switch (test)
        {
        case Tests::First:
            pattern = "\x45\x43\x45\x55\x33\x9a\xfa\xCC\xCC\xCC\xCC\x45\x68\x21";
            break;
        case Tests::Second:
            pattern = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\xCC\xCC\xCC\xCC\x45\x68\x21";
            break;
        default:
            break;
        }
    }

    virtual LPVOID runOne( PBYTE baseAddress, DWORD size )
    {
        return const_cast<LPVOID>(Search( baseAddress, size, reinterpret_cast<const uint8_t*>(pattern), strlen( pattern ), 0xCC ));
    }
    virtual const char* name() const
    {
        return "DarthTon";
    }

    const char* pattern = nullptr;
};

REGISTER( DARTH_TON );

struct PartData
{
    int32_t mask = 0;
    __m128i needle; //C2797: list initialization inside member initializer list or non-static data member initializer is not implemented


    PartData()
    {
        memset(&needle, 0, sizeof(needle));
    }
};

const void* Search( const uint8_t* data, const uint32_t size, const uint8_t* pattern, const char* mask )
{
    const uint8_t* result = nullptr;
    auto len = strlen( mask );
    auto first = strchr( mask, '?' );
    size_t len2 = (first != nullptr) ? (first - mask) : len;
    auto firstlen = min( len2, 16 );
    intptr_t num_parts = (len < 16 || len % 16) ? (len / 16 + 1) : (len / 16);
    PartData parts[4];

    for (intptr_t i = 0; i < num_parts; ++i, len -= 16)
    {
        for (size_t j = 0; j < min( len, 16 ) - 1; ++j)
            if (mask[16 * i + j] == 'x')
                _bittestandset( (LONG*)&parts[i].mask, j );

        parts[i].needle = _mm_loadu_si128( (const __m128i*)(pattern + i * 16) );
    }

    bool abort = false;

#pragma omp parallel for
    for (intptr_t i = 0; i < static_cast<intptr_t>(size) / 32 - 1; ++i)
    {
        #pragma omp flush (abort)
        if(!abort)
        {
            auto block = _mm256_loadu_si256( (const __m256i*)data + i );
            if (_mm256_testz_si256( block, block ))
                continue;

            auto offset = _mm_cmpestri( parts->needle, firstlen, _mm_loadu_si128( (const __m128i*)(data + i * 32) ), 16, _SIDD_CMP_EQUAL_ORDERED );
            if (offset == 16)
            {
                offset += _mm_cmpestri( parts->needle, firstlen, _mm_loadu_si128( (const __m128i*)(data + i * 32 + 16) ), 16, _SIDD_CMP_EQUAL_ORDERED );
                if (offset == 32)
                    continue;
            }

            for (intptr_t j = 0; j < num_parts; ++j)
            {
                auto hay = _mm_loadu_si128( (const __m128i*)(data + (2 * i + j) * 16 + offset) );
                auto bitmask = _mm_movemask_epi8( _mm_cmpeq_epi8( hay, parts[j].needle ) );
                if ((bitmask & parts[j].mask) != parts[j].mask)
                    goto next;
            }

            result = data + 32 * i + offset;
            abort = true;
            #pragma omp flush (abort)
        }
        //break;  //C3010: 'break' : jump out of OpenMP structured block not allowed

    next:;
    }

    return result;
}

struct DARTH_TON2 : public BenchBase
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
        return const_cast<LPVOID>(Search( baseAddress, size, reinterpret_cast<const uint8_t*>(pattern), mask ));
    }
    virtual const char* name() const
    {
        return "DarthTon v2";
    }

    const char* pattern = nullptr;
    const char* mask = nullptr;
};

REGISTER( DARTH_TON2 );

#endif // DARTHTON_H
