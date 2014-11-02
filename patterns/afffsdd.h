#ifndef AFFFSDD_H
#define AFFFSDD_H

bool CompareByteArray(PBYTE Data, PBYTE Signature)
{
	for (; *Signature; ++Signature, ++Data)
	{
		if (*Signature == '\x00')
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

PBYTE FindSignature(PBYTE BaseAddress, DWORD ImageSize, PBYTE Signature)
{
	BYTE First = Signature[0];
	PBYTE Max = BaseAddress + ImageSize - strlen((PCHAR) Signature);

	for (; BaseAddress < Max; ++BaseAddress)
	{
		if (*BaseAddress != First)
		{
			continue;
		}
		if (CompareByteArray(BaseAddress, Signature))
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
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return FindSignature(baseAddress, size, (PBYTE) mPattern);
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
