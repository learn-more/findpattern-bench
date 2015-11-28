#ifndef MICHAEL_H
#define MICHAEL_H

BYTE *SearchSignature(BYTE *start_pos, size_t search_len, const unsigned char *pattern, const unsigned char *mask)
{
    for(BYTE *region_it = start_pos; region_it < (start_pos + search_len); ++region_it)
    {

        if(*region_it == *pattern)
        {
            const unsigned char *pattern_it = pattern,
                *mask_it = mask,
                *memory_it = region_it;
            bool found = true;

            for(; *mask_it && (memory_it < (start_pos + search_len)); ++mask_it, ++pattern_it, ++memory_it)
            {
                if(*mask_it != 'x') continue;
                if(*memory_it != *pattern_it)
                {
                    found = false;
                    break;
                }
            }

            if(found) return region_it;
        }

    }
}

struct MICHAEL : public BenchBase
{
    virtual void init(Tests test)
    {
        switch(test)
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
        return SearchSignature(baseAddress, size, (BYTE*) mPattern, (BYTE*)mMask);
    }

    virtual const char* name() const
    {
        return "Michael";
    }
    PCHAR mPattern; // = reinterpret_cast<PCHAR>("");
    PCHAR mMask; // = reinterpret_cast<PCHAR>("");
};

REGISTER(MICHAEL);

#endif // MICHAEL_H