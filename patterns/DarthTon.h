#ifndef DARTHTON_H
#define DARTHTON_H

// http://www.unknowncheats.me/forum/c-and-c/125497-findpattern-benchmark.html#post1064488

LPVOID Search(uint8_t* pPattern, size_t patternSize, uint8_t wildcard, uint8_t* scanStart, size_t scanSize)
{
	for (uint8_t *ptr = scanStart; ptr < scanStart + scanSize - patternSize; ++ptr)
	{
		for (size_t i = 0; i < patternSize; ++i)
			if (ptr[i] != pPattern[i] && pPattern[i] != wildcard)
				goto end;

		return ptr;
	end:;
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
			pattern = "\x45\x43\x45\x55\x33\x9a\xfa\xcc\xcc\xcc\xcc\x45\x68\x21";
			wildcard = 0xCC;
			break;
		case Tests::Second:
			pattern = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\xcc\xcc\xcc\xcc\x45\x68\x21";
			wildcard = 0xCC;
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return Search(reinterpret_cast<uint8_t*>(pattern), strlen(pattern), wildcard, baseAddress, size);
	}
	virtual const char* name() const
	{
		return "DarthTon";
	}

	char* pattern = "";
	uint8_t wildcard = 0;
};

REGISTER(DARTH_TON);

#endif // DARTHTON_H
