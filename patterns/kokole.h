#ifndef KOKOLE_H
#define KOKOLE_H

bool DataCompare(BYTE* pData, BYTE* bSig, char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig)
	{
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

BYTE* FindPattern(BYTE* dwAddress, DWORD dwSize, BYTE* pbSig, char* szMask)
{
	DWORD length = strlen(szMask);
	for (DWORD i = NULL; i < dwSize - length; i++)
	{
		if (DataCompare(dwAddress + i, pbSig, szMask))
			return dwAddress + i;
	}
	return 0;
}

struct KOKOLE : public PatternScanner
{
	virtual void init(Pattern* pattern)
	{
        mPattern = pattern->raw;
        mMask = pattern->mask;
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return FindPattern(baseAddress, size, (BYTE*)mPattern, mMask);
	}

	virtual const char* name() const
	{
		return "kokole";
	}
	PCHAR mPattern; // = reinterpret_cast<PCHAR>("");
	PCHAR mMask; // = reinterpret_cast<PCHAR>("");
};

REG_SCAN(KOKOLE);

#endif // KOKOLE_H
