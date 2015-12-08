#ifndef DARTHTON_H
#define DARTHTON_H

// Boyer-Moore-Horspool with wildcards implementation
inline LPVOID Search( uint8_t* pPattern, size_t patternSize, uint8_t wildcard, uint8_t* pScanPos, size_t scanSize )
{
    size_t bad_char_skip[UCHAR_MAX + 1];
    uint8_t* scanEnd = pScanPos + scanSize - patternSize;
    size_t idx = 0;
    size_t last = patternSize - 1;

    // Get last wildcard position
    for (idx = last; idx > 0 && pPattern[idx] != wildcard; --idx);
    size_t diff = patternSize - idx;

    // Prepare shift table
    for (idx = 0; idx <= UCHAR_MAX; ++idx)
        bad_char_skip[idx] = diff;
    for (idx = 0; idx < last; ++idx)
        bad_char_skip[pPattern[idx]] = last - idx;

    // Search
    for (; pScanPos < scanEnd; pScanPos += bad_char_skip[pScanPos[last]])
    {
        for (idx = last; idx > 0; --idx)
            if (pPattern[idx] != wildcard && pScanPos[idx] != pPattern[idx])
                goto skip;

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
        return Search( reinterpret_cast<uint8_t*>(pattern), strlen( pattern ), 0xCC, baseAddress, size );
    }
    virtual const char* name() const
    {
        return "DarthTon";
    }

    char* pattern = "";
};

REGISTER( DARTH_TON );

struct PartData
{
    int32_t size = 0;
    int32_t mask = 0;
    __m128i needle = { 0 };
};

const void* Search( const uint8_t* data, const uint32_t size, const uint8_t* pattern, const char* mask )
{
    auto len = strlen( mask );
    intptr_t num_parts = (len < 16 || len % 16) ? (len / 16 + 1) : (len / 16);
    PartData parts[32];
    const uint8_t* result = nullptr;

    for (intptr_t i = 0; i < num_parts; ++i)
    {
        parts[i].size = static_cast<int32_t>(strlen( mask + i * 16 ));
        parts[i].needle = _mm_loadu_si128( (const __m128i*)(pattern + i * 16) );
        for (intptr_t j = parts[i].size - 1; j >= 0; --j)
            if (mask[i * 16 + j] == 'x')
                parts[i].mask |= 1 << (j % 16);
    }

#pragma omp parallel for
    for (intptr_t i = 0; i < static_cast<intptr_t>(size) / 32 - 1; ++i)
    {
        auto block = _mm256_loadu_si256( (const __m256i*)data + i );
        if (_mm256_testz_si256( block, block ))
            continue;

        auto offset = _mm_cmpestri( parts[0].needle, parts[0].size, _mm_loadu_si128( (const __m128i*)(data + i * 32) ), 16, _SIDD_CMP_EQUAL_ORDERED );
        if (offset == 16)
        {
            offset = _mm_cmpestri( parts[0].needle, parts[0].size, _mm_loadu_si128( (const __m128i*)(data + i * 32 + 16) ), 16, _SIDD_CMP_EQUAL_ORDERED );
            if (offset == 16)
                continue;

            offset += 16;
        }

        for (intptr_t j = 0; j < num_parts; ++j)
        {
            auto hay = _mm_loadu_si128( (const __m128i*)(data + (2 * i + j) * 16 + offset) );
            auto bitmask = _mm_movemask_epi8( _mm_cmpeq_epi8( hay, parts[j].needle ) );
            if ((bitmask & parts[j].mask) != parts[j].mask)
                goto next;
        }

        result = data + i * 32 + offset;
        break;
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
        return const_cast<LPVOID>( Search( baseAddress, size, reinterpret_cast<uint8_t*>(pattern), mask ) );
    }
    virtual const char* name() const
    {
        return "DarthTon v2";
    }

    char* pattern = "";
    char* mask = "";
};

REGISTER( DARTH_TON2 );

#endif // DARTHTON_H
