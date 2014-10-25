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


struct DARTH_TON : public BenchBase
{
	virtual void init(Tests test)
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

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
        return Search( reinterpret_cast<uint8_t*>(pattern), strlen( pattern ), 0xCC, baseAddress, size );
	}
	virtual const char* name() const
	{
		return "DarthTon";
	}

	char* pattern = "";
};

REGISTER(DARTH_TON);

#endif // DARTHTON_H
