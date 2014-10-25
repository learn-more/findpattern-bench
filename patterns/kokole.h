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

struct KOKOLE : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
			mMask = "xxxxxxx????xxx";
			break;
		case Tests::Second:
			mPattern = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
			mMask = "xxxxxxxxxxx????xxx";
			break;
		default:
			break;
		}
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

REGISTER(KOKOLE);

#endif // KOKOLE_H
