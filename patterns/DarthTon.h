#ifndef DARTHTON_H
#define DARTHTON_H

// http://www.unknowncheats.me/forum/c-and-c/125497-findpattern-benchmark.html#post1065271
void FillShiftTable( uint8_t* pPattern, size_t patternSize, uint8_t wildcard, size_t* bad_char_skip )
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

// Boyer-Moore-Horspool with wildcards implementation
LPVOID Search( uint8_t* pScanPos, size_t scanSize, uint8_t* pPattern, size_t patternSize, uint8_t wildcard )
{
    size_t bad_char_skip[UCHAR_MAX + 1];
    uint8_t* scanEnd = pScanPos + scanSize - patternSize;
    size_t idx = 0;
    size_t last = patternSize - 1;

    FillShiftTable( pPattern, patternSize, wildcard, bad_char_skip );

    // Search
    for (; pScanPos <= scanEnd; pScanPos += bad_char_skip[pScanPos[last]])
    {
        for (idx = last; idx >= 0; --idx)
            if (pPattern[idx] != wildcard && pScanPos[idx] != pPattern[idx])
                goto skip;
            else if (idx == 0)
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
        return Search( baseAddress, size, reinterpret_cast<uint8_t*>(_pattern), len, wildcard );
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
