#ifndef LEARN_MORE_H
#define LEARN_MORE_H

// http://www.unknowncheats.me/forum/c-and-c/77419-findpattern.html#post650040
// Original code by learn_more
// Fix based on suggestion from stevemk14ebr : http://www.unknowncheats.me/forum/1056782-post13.html

#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

namespace
{

	PBYTE findPattern(const PBYTE rangeStart, const PBYTE rangeEnd, const char* pattern)
	{
		const unsigned char* pat = reinterpret_cast<const unsigned char*>(pattern);
		PBYTE firstMatch = 0;
		for (PBYTE pCur = rangeStart; pCur < rangeEnd; ++pCur) {
			if (*(PBYTE)pat == (BYTE)'\?' || *pCur == getByte(pat)) {
				if (!firstMatch) {
					firstMatch = pCur;
				}
				pat += (*(PWORD)pat == (WORD)'\?\?' || *(PBYTE)pat != (BYTE)'\?') ? 3 : 2;
				if (!*pat) {
					return firstMatch;
				}
			} else if (firstMatch) {
				pCur = firstMatch;
				pat = reinterpret_cast<const unsigned char*>(pattern);
				firstMatch = 0;
			}
		}
		return NULL;
	}

	struct LM : public PatternScanner
	{
		virtual void init(Pattern* pattern)
		{
            mPattern = pattern->ida;
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


	REG_SCAN(LM);

} // namespace


namespace
{

	inline
		bool isMatch(const PBYTE addr, const PBYTE pat, const PBYTE msk)
	{
			size_t n = 0;
			while (addr[n] == pat[n] || msk[n] == (BYTE)'?') {
				if (!msk[++n]) {
					return true;
				}
			}
			return false;
	}

	PBYTE findPattern_v2(const PBYTE rangeStart, DWORD len, const char* pattern)
	{
		size_t l = strlen(pattern);
		PBYTE patt_base = static_cast<PBYTE>(_alloca(l >> 1));
		PBYTE msk_base = static_cast<PBYTE>(_alloca(l >> 1));
		PBYTE pat = patt_base;
		PBYTE msk = msk_base;
		l = 0;
		while (*pattern) {
			if (*(PBYTE)pattern == (BYTE)'\?') {
				*pat++ = 0;
				*msk++ = '?';
				pattern += ((*(PWORD)pattern == (WORD)'\?\?') ? 3 : 2);
			} else {
				*pat++ = getByte(pattern);
				*msk++ = 'x';
				pattern += 3;
			}
			l++;
		}
		*msk = 0;
		pat = patt_base;
		msk = msk_base;
		for (DWORD n = 0; n < (len - l); ++n)
		{
			if (isMatch(rangeStart + n, patt_base, msk_base)) {
				return rangeStart + n;
			}
		}
		return NULL;
	}

    struct LM2: public PatternScanner
	{
        virtual void init( Pattern* pattern )
        {
            mPattern = pattern->ida;
        }

		virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
		{
			return findPattern_v2(baseAddress, size, mPattern);
		}
		virtual const char* name() const
		{
			return "learn_more v2";
		}
		const char* mPattern = "";
	};


    REG_SCAN( LM2 );

} // namespace





#endif // LEARN_MORE_H
