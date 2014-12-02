#ifndef TRIPPEH_H
#define TRIPPEH_H

// http://www.unknowncheats.me/forum/c-and-c/125426-my-findpattern-function.html


namespace
{

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

    struct TRIPPEH: public PatternScanner
	{
		virtual void init(Pattern* pattern)
		{
            mPattern = pattern->hybrid;
            mMask = pattern->mask;
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

    REG_SCAN( TRIPPEH );

} // namespace


namespace ns_TRIPPEH_V2
{
    // Update: DarthTon: added 'wildcard' for proper wildcard support
	BOOL CompareByteArray(PBYTE ByteArray1, PBYTE ByteArray2, DWORD Length, BYTE wildcard)
	{
		for (DWORD i = 0; i < Length; i++)
		{
            if (ByteArray2[i] != wildcard && ByteArray1[i] != ByteArray2[i])
			{
				return FALSE;
			}
		}

		return TRUE;
	}

    // Update: DarthTon: added 'wildcard' for proper wildcard support
	PBYTE GetAddress(PBYTE BaseAddress, DWORD ImageSize, PBYTE ByteArray, DWORD Length, BYTE wildcard)
	{
		PBYTE Address = 0x00000000;

		for (DWORD i = 0; i < (ImageSize - Length); i++)
		{
			if (CompareByteArray((BaseAddress + i), ByteArray, Length, wildcard))
			{
				Address = (PBYTE)BaseAddress + i;
				break;
			}
		}

		return Address;
	}

    struct TRIPPEH_V2: public PatternScanner
	{
        virtual void init( Pattern* pattern )
        {
            mPattern = (PBYTE)pattern->raw;
            mLength = pattern->length;
            mWildcard = pattern->wildcard;
        }

		virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
		{
			return GetAddress(baseAddress, size, mPattern, mLength, mWildcard);
		}
		virtual const char* name() const
		{
			return "Trippeh v2";
		}
		PBYTE mPattern;
		DWORD mLength;
        BYTE mWildcard;
	};

    REG_SCAN( TRIPPEH_V2 );

} // namespace


namespace ns_TRIPPEH_V3
{

	BOOL CompareByteArray(PBYTE ByteArray1, PBYTE ByteArray2, DWORD Size, BYTE Wildcard)
	{
		for (DWORD i = 0; i < Size; i++)
		{
			if (ByteArray2[i] != Wildcard && ByteArray1[i] != ByteArray2[i])
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	PBYTE FindPattern(PBYTE BaseAddress, DWORD ImageSize, PCHAR Pattern, DWORD Size, BYTE Wildcard, DWORD Skip)
	{
		PBYTE ByteArray = new BYTE[Size];

		for (DWORD i = 0; i < Size; i++)
		{
			ByteArray[i] = (BYTE)strtol(Pattern, &Pattern, 16);
		}

		PBYTE Address = 0;

		for (DWORD i = 0; i < ImageSize - Size; i += Skip)
		{
			if (CompareByteArray((BaseAddress + i), ByteArray, Size, Wildcard))
			{
				Address = BaseAddress + i;
				break;
			}
		}

		delete[] ByteArray;

		return Address;
	}

    struct TRIPPEH_V3: public PatternScanner
	{
        virtual void init( Pattern* pattern )
        {
            m_pattern = pattern->ida;
            m_size = pattern->length;
            m_wildcard = pattern->wildcard;

            switch (pattern->index)
            {
                case 0:
                    m_skip = 4;
                    break;
                case 1:
                    m_skip = 2;
                    break;

                    // FIXME: DarthTon: I have no idea how 'm_skip' value is deducted
                case 2:
                    m_skip = 1;
                    break;
                case 3:
                    m_skip = 1;
                    break;
                default:
                    break;
            }
        }


		virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
		{
			return FindPattern(baseAddress, size, m_pattern, m_size, m_wildcard, m_skip);
		}
		virtual const char* name() const
		{
			return "Trippeh v3";
		}

		PCHAR m_pattern;
		DWORD m_size;
		DWORD m_skip;
        BYTE m_wildcard;
	};

    REG_SCAN( TRIPPEH_V3 );

    size_t *shift_table( unsigned char *needle, size_t nlen, size_t skip )
    {
        size_t scan = 0;
        size_t bad_char_skip[UCHAR_MAX + 1];
        size_t last = nlen - 1;

        for (scan = 0; scan <= UCHAR_MAX; scan++)
            bad_char_skip[scan] = skip;

        for (scan = 0; scan < last; scan++)
            bad_char_skip[needle[scan]] = last - scan;

        return bad_char_skip;
    }

    unsigned char *boyer_moore_horspool( unsigned char *haystack, size_t hlen, unsigned char *needle, size_t nlen, unsigned char wildcard, size_t skip )
    {
        size_t scan = 0;
        size_t *bad_char_skip = shift_table( needle, nlen, skip );
        size_t last = nlen - 1;

        while (hlen >= nlen)
        {
            for (scan = last; scan > 0; scan--)
            {
                if (needle[scan] != wildcard && haystack[scan] != needle[scan])
                    break;

                return haystack;
            }

            hlen -= bad_char_skip[haystack[last]];
            haystack += bad_char_skip[haystack[last]];
        }

        return 0;
    }

    struct TRIPPEH_V4: public PatternScanner
    {
        virtual void init( Pattern* pattern )
        {
            m_pattern = pattern->raw;
            m_len = pattern->length;
            m_wildcard = pattern->wildcard;
        }

        virtual LPVOID runOne( PBYTE baseAddress, DWORD size )
        {
            return boyer_moore_horspool( baseAddress, size, (unsigned char *)m_pattern, m_len, m_wildcard, 4 );
        }
        virtual const char* name() const
        {
            return "Trippeh v4";
        }

        char *m_pattern;
        size_t m_len;
        BYTE m_wildcard;
    };

    REG_SCAN( TRIPPEH_V4 );

} // namespace

#endif // TRIPPEH_H
