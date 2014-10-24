#ifndef AFFFSDD_H
#define AFFFSDD_H

bool CompareByteArray(PBYTE ByteArray1, PCHAR ByteArray2,  DWORD Length)
{
	for (DWORD i = 0; i < Length; i++)
	{
		if (ByteArray2[i] == '\x00')
		{
			continue;
		}
		if (ByteArray1[i] != (BYTE)ByteArray2[i])
		{
			return false;
		}
	}
	return true;
}

PBYTE FindSignature(LPVOID BaseAddress, DWORD ImageSize, PCHAR Signature)
{
	PBYTE Address = NULL;
	PBYTE Buffer = (PBYTE)BaseAddress;

	DWORD Length = strlen(Signature);

	for (DWORD i = 0; i < (ImageSize - Length); i++)
	{
		if (CompareByteArray((Buffer + i), Signature, Length))
		{
			Address = (PBYTE)BaseAddress + i;
			break;
		}
	}
	return Address;
}


struct AFFFSDD : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
			//mMask = "xxxxxxx????xxx";
			break;
		case Tests::Second:
			mPattern = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
			//mMask = "xxxxxxxxxxx????xxx";
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return FindSignature(baseAddress, size, mPattern);
	}

	virtual const char* name() const
	{
		return "afffsdd";
	}
	PCHAR mPattern = reinterpret_cast<PCHAR>("");
	PCHAR mMask = reinterpret_cast<PCHAR>("");
};

REGISTER(AFFFSDD);

#endif // AFFFSDD_H
