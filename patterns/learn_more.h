#ifndef LEARN_MORE_H
#define LEARN_MORE_H

// http://www.unknowncheats.me/forum/c-and-c/77419-findpattern.html#post650040

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

PBYTE findPattern(PBYTE rangeStart, PBYTE rangeEnd, const char* pattern)
{
	const char* pat = pattern;
	PBYTE firstMatch = 0;
	for (PBYTE pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == '\?' || *pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
			else pat += 2;    //one ?
		} else {
			pat = pattern;
			firstMatch = 0;
		}
	}
	return NULL;
}

struct LM : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = "45 43 45 55 33 9a fa ? ? ? ? 45 68 21";
			break;
		case Tests::Second:
			mPattern = "aa aa aa aa aa aa aa aa aa bb aa ? ? ? ? 45 68 21";
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return findPattern(baseAddress, baseAddress + size, mPattern);
	}
	virtual const char* name() const
	{
		return "learn_more";
	}
	const char* mPattern = "";
};


REGISTER(LM);

#endif // LEARN_MORE_H
