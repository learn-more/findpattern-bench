#ifndef MREXODIA_H
#define MREXODIA_H

//https://bitbucket.org/mrexodia/patternfind

using namespace std;

struct PatternByte
{
	struct PatternNibble
	{
		unsigned char data;
		bool wildcard;
	} nibble[2];
};

static string formathexpattern(string patterntext)
{
	string result;
	int len = patterntext.length();
	for (int i = 0; i < len; i++)
		if (patterntext[i] == '?' || isxdigit(patterntext[i]))
			result += toupper(patterntext[i]);
	return result;
}

static int hexchtoint(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	return 0;
}

static bool patterntransform(string patterntext, vector<PatternByte> & pattern)
{
	pattern.clear();
	patterntext = formathexpattern(patterntext);
	int len = patterntext.length();
	if (!len)
		return false;

	if (len % 2) //not a multiple of 2
	{
		patterntext += '?';
		len++;
	}

	PatternByte newByte;
	for (int i = 0, j = 0; i < len; i++)
	{
		if (patterntext[i] == '?') //wildcard
		{
			newByte.nibble[j].wildcard = true; //match anything
		}
		else //hex
		{
			newByte.nibble[j].wildcard = false;
			newByte.nibble[j].data = hexchtoint(patterntext[i]) & 0xF;
		}

		j++;
		if (j == 2) //two nibbles = one byte
		{
			j = 0;
			pattern.push_back(newByte);
		}
	}
	return true;
}

static bool patternmatchbyte(unsigned char byte, const PatternByte & pbyte)
{
	int matched = 0;

	unsigned char n1 = (byte >> 4) & 0xF;
	if (pbyte.nibble[0].wildcard)
		matched++;
	else if (pbyte.nibble[0].data == n1)
		matched++;

	unsigned char n2 = byte & 0xF;
	if (pbyte.nibble[1].wildcard)
		matched++;
	else if (pbyte.nibble[1].data == n2)
		matched++;

	return (matched == 2);
}

size_t patternfind(unsigned char* data, size_t datasize, const char* pattern)
{
	vector<PatternByte> searchpattern;
	if (!patterntransform(pattern, searchpattern))
		return -1;
	size_t searchpatternsize = searchpattern.size();
	for (size_t i = 0, pos = 0; i < datasize; i++) //search for the pattern
	{
		if (patternmatchbyte(data[i], searchpattern.at(pos))) //check if our pattern matches the current byte
		{
			pos++;
			if (pos == searchpatternsize) //everything matched
				return i - searchpatternsize + 1;
		}
		else if (pos > 0) //fix by Computer_Angel
		{
			i -= pos;
			pos = 0; //reset current pattern position
		}
	}
	return -1;
}

struct MREXODIA : public BenchBase
{
	virtual void init(Tests test)
	{
		switch (test)
		{
		case Tests::First:
			mPattern = "45434555339AFA????????456821";
			break;
		case Tests::Second:
			mPattern = "AAAAAAAAAAAAAAAAAABBAA????????456821";
			break;
		default:
			break;
		}
	}

	virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
	{
		size_t found = patternfind((unsigned char*)baseAddress, size, mPattern);
		return (LPVOID)(found + (size_t)baseAddress);
	}

	virtual const char* name() const
	{
		return "mrexodia";
	}
	PCHAR mPattern; // = reinterpret_cast<PCHAR>("");
};

REGISTER(MREXODIA);

#endif