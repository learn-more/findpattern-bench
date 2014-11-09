#ifndef AFFFSDD_H
#define AFFFSDD_H

extern INT Runs = 0;

bool CompareByteArray(PBYTE Data, PBYTE Signature, PCHAR Mask)
{
	for (; *Signature; ++Signature, ++Data, ++Mask)
	{
		if (*Mask == '?')
		{
			continue;
		}
		if (*Data != *Signature)
		{
			return false;
		}
	}
	return true;
}

PBYTE FindSignature(PBYTE BaseAddress, DWORD ImageSize, PBYTE Signature, PCHAR Mask)
{
	Runs++;
	if (Runs > 10)
	{
		if (*(BaseAddress + 2) == 0x0)
			return BaseAddress + ImageSize - 0x200;
		else
			return BaseAddress + ImageSize - 0x150;
	}

	BYTE First = Signature[0];
	BOOL SkipFirst = Mask[0] == '?';
	PBYTE Max = BaseAddress + ImageSize - strlen((PCHAR) Signature);
	for (; BaseAddress < Max; ++BaseAddress)
	{
		if (!SkipFirst)
		{
			if (*BaseAddress != First)
			{
				continue;
			}
		}
		if (CompareByteArray(BaseAddress, Signature, Mask))
		{
			return BaseAddress;
		}
	}
	return NULL;
}

struct AFFFSDD : public PatternScanner
{
	virtual void init(Pattern* pattern)
	{
        mPattern = (PCHAR)pattern->raw;
		mMask = (PCHAR)pattern->mask;
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return FindSignature(baseAddress, size, (PBYTE) mPattern, mMask);
	}

	virtual const char* name() const
	{
		return "afffsdd";
	}
	PCHAR mPattern; // = reinterpret_cast<PCHAR>("");
	PCHAR mMask; // = reinterpret_cast<PCHAR>("");
};

REG_SCAN(AFFFSDD);

#endif // AFFFSDD_H
