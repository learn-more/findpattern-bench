#ifndef ATOM0S_H
#define ATOM0S_H

//http://atom0s.com/forums/viewtopic.php?f=5&t=4

/**
 * @brief Scans a given chunk of data for the given pattern and mask.
 *
 * @param data          The data to scan within for the given pattern.
 * @param baseAddress   The base address of where the scan data is from.
 * @param lpPattern     The pattern to scan for.
 * @param pszMask       The mask to compare against for wildcards.
 * @param offset        The offset to add to the pointer.
 * @param resultUsage   The result offset to use when locating signatures that match multiple functions.
 *
 * @return Pointer of the pattern found, 0 otherwise.
 */
static intptr_t FindPattern(std::vector<unsigned char> data, intptr_t baseAddress, const unsigned char* lpPattern, const char* pszMask, intptr_t offset, intptr_t resultUsage)
{
	// Build vectored pattern.. 
	std::vector<std::pair<unsigned char, bool>> pattern;
	for (size_t x = 0, y = strlen(pszMask); x < y; x++)
		pattern.push_back(std::make_pair(lpPattern[x], pszMask[x] == 'x'));

	auto scanStart = data.begin();
	auto resultCnt = 0;

	while (true)
	{
		// Search for the pattern.. 
		auto ret = std::search(scanStart, data.end(), pattern.begin(), pattern.end(),
			[&](unsigned char curr, std::pair<unsigned char, bool> currPattern)
		{
			return (!currPattern.second) || curr == currPattern.first;
		});

		// Did we find a match.. 
		if (ret != data.end())
		{
			// If we hit the usage count, return the result.. 
			if (resultCnt == resultUsage || resultUsage == 0)
				return (std::distance(data.begin(), ret) + baseAddress) + offset;

			// Increment the found count and scan again.. 
			++resultCnt;
			scanStart = ++ret;
		}
		else
			break;
	}

	return 0;
}

struct ATOM0S : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = (const unsigned char*)"\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
			mMask = "xxxxxxx????xxx";
			break;
		case Tests::Second:
			mPattern = (const unsigned char*)"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
			mMask = "xxxxxxxxxxx????xxx";
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		std::vector<unsigned char> rawData((unsigned char*)baseAddress, (unsigned char*)baseAddress + size);
		return (LPVOID)FindPattern(rawData, (intptr_t)baseAddress, mPattern, mMask, 0, 0);
	}

	virtual const char* name() const
	{
		return "atom0s";
	}

	const unsigned char* mPattern; // = reinterpret_cast<PCHAR>("");
	const char* mMask;
};

REGISTER(ATOM0S);

#endif
