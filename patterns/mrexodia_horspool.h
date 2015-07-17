#ifndef MREXODIA_HORSPOOL_H
#define MREXODIA_HORSPOOL_H

//based on: https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm

bool matches(const unsigned char haystack_ch, const unsigned char needle_ch, const unsigned char wildcard)
{
	return needle_ch == wildcard || haystack_ch == needle_ch;
}

const unsigned char *
boyermoore_horspool_memmem(const unsigned char* haystack, size_t hlen,
const unsigned char* needle, size_t nlen,
const unsigned char wildcard = '?')
{
	size_t bad_char_skip[UCHAR_MAX + 1]; /* Officially called: bad character shift */

	/* Sanity checks on the parameters */
	if (nlen <= 0 || !haystack || !needle)
		return NULL;

	/* ---- Preprocess ---- */
	/* Initialize the table to default value */
	/* When a character is encountered that does not occur
	* in the needle, we can safely skip ahead for the whole
	* length of the needle.
	*/
	for (size_t scan = 0; scan <= UCHAR_MAX; scan = scan + 1)
	{
		bad_char_skip[scan] = nlen;
	}

	/* C arrays have the first byte at [0], therefore:
	* [nlen - 1] is the last byte of the array. */
	size_t last = nlen - 1;

	/* Then populate it with the analysis of the needle */
	for (size_t scan = 0; scan < last; scan = scan + 1)
	{
		unsigned char needleByte = needle[scan];
		bad_char_skip[needleByte] = last - scan;
	}

	/* ---- Do the matching ---- */

	/* Search the haystack, while the needle can still be within it. */
	while (hlen >= nlen)
	{
		/* scan from the end of the needle */
		for (size_t scan = last; matches(haystack[scan], needle[scan], wildcard); scan = scan - 1)
		{
			if (scan == 0) /* If the first byte matches, we've found it. */
				return haystack;
		}

		/* otherwise, we need to skip some bytes and start again.
		Note that here we are getting the skip value based on the last byte
		of needle, no matter where we didn't match. So if needle is: "abcd"
		then we are skipping based on 'd' and that value will be 4, and
		for "abcdd" we again skip on 'd' but the value will be only 1.
		The alternative of pretending that the mismatched character was
		the last character is slower in the normal case (E.g. finding
		"abcd" in "...azcd..." gives 4 by using 'd' but only
		4-2==2 using 'z'. */
		unsigned char lastByte = haystack[last];
		hlen -= bad_char_skip[lastByte];
		haystack += bad_char_skip[lastByte];
	}

	return NULL;
}

struct MREXODIA_HORSPOOL : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = (unsigned char*)"\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21";
			mLen = 14;
			break;
		case Tests::Second:
			mPattern = (unsigned char*)"\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21";
			mLen = 18;
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		return (LPVOID)boyermoore_horspool_memmem((unsigned char*)baseAddress, size, mPattern, mLen, 0x00);
	}

	virtual const char* name() const
	{
		return "mrexodia (horspool)";
	}

	unsigned char* mPattern;
	int mLen;
};

REGISTER(MREXODIA_HORSPOOL);

#endif