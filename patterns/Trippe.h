#ifndef TRIPPEH_H
#define TRIPPEH_H

// http://www.unknowncheats.me/forum/c-and-c/125426-my-findpattern-function.html

enum Radix : int
{
	Hexadecimal = 16
};


BOOL CompareByteArray(PBYTE ByteArray1, PCHAR ByteArray2, PCHAR Mask, DWORD Length)
{
	for (DWORD i = 0; i < Length; i++)
	{
		if (ByteArray1[i] != (BYTE)strtol(ByteArray2, &ByteArray2, Hexadecimal) && Mask[i] != '?')
		{
			return FALSE;
		}
	}

	return TRUE;
}

PBYTE GetAddress(LPVOID BaseAddress, DWORD ImageSize, PBYTE Buffer, PCHAR ByteArray, PCHAR Mask)
{
	PBYTE Address = 0x00000000;
	DWORD Length = strlen(Mask);

	for (DWORD i = 0; i < (ImageSize - Length); i++)
	{
		if (CompareByteArray((Buffer + i), ByteArray, Mask, Length))
		{
			Address = (PBYTE)BaseAddress + i;
			break;
		}
	}

	return Address;
}

struct TRIPPEH : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = "45 43 45 55 33 9a fa 00 00 00 00 45 68 21";
			mMask = "xxxxxxx????xxx";
			break;
		case Tests::Second:
			mPattern = "aa aa aa aa aa aa aa aa aa bb aa 00 00 00 00 45 68 21";
			mMask = "xxxxxxxxxxx????xxx";
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return GetAddress(baseAddress, size, baseAddress, mPattern, mMask);
	}
	virtual const char* name() const
	{
		return "Trippeh";
	}
	PCHAR mPattern = "";
	PCHAR mMask = "";
};

REGISTER(TRIPPEH);





#endif // TRIPPEH_H
