#ifndef M_I_K_E_H
#define M_I_K_E_H

// http://www.unknowncheats.me/forum/americas-army-proving-grounds/115761-m-k-es-aa-pg-sdk-7-jun-2014-a.html#post979201
// Issues:
// patterns are case / formatting sensitive.
// original code misses define for HEX_NEG_MASK
// original code runs too far (the recursive bit)
// original code returns one byte into the pattern.


DWORD FindSignature( DWORD base, DWORD size, unsigned char* pattern, unsigned int &instances )
{
	//unsigned int opcodes = 0;			// edit out by lm
	unsigned char *code = (unsigned char *)base;
	int patternLength = strlen( (char *)pattern );

	if( pattern[ 0 ] == ' ' )
		return NULL;

	// edit out by lm: HEX_NEG_MASK was nowhere to be found :)
	//if( (size & Constants::HEX_NEG_MASK) ||
	//	(base & Constants::HEX_NEG_MASK) )
	//	return NULL;

	//opcodes = GetNoOpcodes( (const char *)pattern );

	//if( (size - opcodes) & Constants::HEX_NEG_MASK )
	//	return NULL;
	// end edit by lm.

	for( unsigned int i(0); i < size; i++ )
	{
		for ( int j(0), k(0); j < patternLength && (i + k < size); k++ )
		{
			unsigned char tempChar[ 3 ];
			memset( tempChar, 0, sizeof( tempChar ) );

			if ( pattern[ j ] == (unsigned char)'?' )
			{
				j += 2;
				continue;
			}

			sprintf( (char *)tempChar, "%02X", code[(i + k)] );

			if( tempChar[ 0 ] != pattern[ j ] ||
				tempChar[ 1 ] != pattern[ j + 1 ] )
				break;

			j += 3;

			if( j > (patternLength - 2) )
			{
				DWORD pointerLoc = (base + i + 1);

				instances++;
				// edited by lm: FindSignature would run outside of bounds, pointerLoc is off-by one.
				//FindSignature( pointerLoc, (size - pointerLoc), pattern, instances );
				--pointerLoc;
				// end edit by lm

				return pointerLoc;
			}
		}
	}

	return NULL;
}

struct MIKE : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = reinterpret_cast<unsigned char*>("45 43 45 55 33 9A FA ? ? ? ? 45 68 21");
			break;
		case Tests::Second:
			mPattern = reinterpret_cast<unsigned char*>("AA AA AA AA AA AA AA AA AA BB AA ? ? ? ? 45 68 21");
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		unsigned int instances = 0;
		return reinterpret_cast<LPVOID>(FindSignature(reinterpret_cast<DWORD>(baseAddress), size, mPattern, instances));
	}
	virtual const char* name() const
	{
		return "M-i-K-e";
	}
	unsigned char* mPattern = reinterpret_cast<unsigned char*>("");
};


REGISTER(MIKE);

#endif // M_I_K_E_H
