#ifndef DARTHTON_H
#define DARTHTON_H

// http://www.unknowncheats.me/forum/c-and-c/125497-findpattern-benchmark.html#post1064488

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


struct DARTH_TON : public PatternScanner
{
    virtual void init( Pattern* pattern )
	{
        _pattern = pattern->raw;
        len = pattern->length;
        wildcard = pattern->wildcard;
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
        return Search( reinterpret_cast<uint8_t*>(_pattern), len, wildcard, baseAddress, size );
	}
	virtual const char* name() const
	{
		return "DarthTon";
	}

    char* _pattern;
    size_t len;
    unsigned char wildcard;
};

REG_SCAN(DARTH_TON);

#endif // DARTHTON_H
