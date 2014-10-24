#ifndef DARTHTON_H
#define DARTHTON_H


LPVOID Search(char* pPattern, size_t patternSize, uint8_t wildcard, uint8_t* scanStart, size_t scanSize)
{
	auto res = std::search(
		scanStart, scanStart + scanSize, pPattern, pPattern + patternSize,
		[&wildcard](uint8_t val1, uint8_t val2) { return (val1 == val2 || val2 == wildcard); }
	);

	return (res >= scanStart + scanSize) ? nullptr : res;
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
		return Search(pattern, strlen(pattern), wildcard, baseAddress, size);
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
