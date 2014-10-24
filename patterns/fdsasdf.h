#ifndef FDSASDF_H
#define FDSASDF_H

int CompareByteArray(PBYTE ByteArray1, PCHAR ByteArray2, PCHAR Mask, DWORD Length)
{
	DWORD nextStart = 0;
	char start = ByteArray2[0];
	for (DWORD i = 0; i < Length; i++)
	{
		if (Mask[i] == '?')
		{
			continue;
		}
		if (ByteArray1[i] == start)
		{
			nextStart = i;
		}
		if (ByteArray1[i] != (BYTE)ByteArray2[i])
		{
			return nextStart;
		}
	}
	return -1;
}

PBYTE FindSignature(LPVOID BaseAddress, DWORD ImageSize, PCHAR Signature, PCHAR Mask)
{
	PBYTE Address = NULL;
	PBYTE Buffer = (PBYTE)BaseAddress;

	DWORD Length = strlen(Mask);

	for (DWORD i = 0; i < (ImageSize - Length); i++)
	{
		int result = CompareByteArray((Buffer + i), Signature, Mask, Length);
		if (result < 0)
		{
			Address = (PBYTE)BaseAddress + i;
			break;
		} else
		{
			i += result;
		}
	}
	return Address;
}


struct FDSASDF : public BenchBase
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
		return FindSignature(baseAddress, size, mPattern, mMask);
	}

	virtual const char* name() const
	{
		return "fdsasdf";
	}
	PCHAR mPattern = reinterpret_cast<PCHAR>("");
	PCHAR mMask = reinterpret_cast<PCHAR>("");
};

REGISTER(FDSASDF)

#endif // FDSASDF_H
