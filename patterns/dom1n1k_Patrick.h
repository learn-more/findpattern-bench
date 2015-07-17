#ifndef DOM1N1K_PATRICK_H
#define DOM1N1K_PATRICK_H

//http://atom0s.com/forums/viewtopic.php?f=5&t=4

bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
    for (; *szMask; ++szMask, ++pData, ++bMask)
        if (*szMask == 'x' && *pData != *bMask)
			return false;
    return (*szMask) == NULL;
}
DWORD Pattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, const char * szMask)
{
	int maskLen = strlen(szMask);
    for (DWORD i = 0; i < dwLen - maskLen; i++)
        if (Compare((BYTE*)(dwAddress + i), bMask, szMask))
			return (DWORD)(dwAddress + i);
    return 0;
}

struct DOM1N1K_PATRICK : public BenchBase
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
		return (LPVOID)Pattern((DWORD)baseAddress, size, (BYTE*)mPattern, mMask);
	}

	virtual const char* name() const
	{
		return "dom1n1k_Patrick";
	}

	const unsigned char* mPattern; // = reinterpret_cast<PCHAR>("");
	const char* mMask;
};

REGISTER(DOM1N1K_PATRICK);

#endif
