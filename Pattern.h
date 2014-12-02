#ifndef PATTERN_H
#define PATTERN_H

struct Pattern
{
    char raw[128];
    char ida[128];
    char hybrid[128];   // specifically for Trippeh V1 scanner
    char mask[128];
    int length;
    unsigned char wildcard;
    int index;

    Pattern( int index_, const char* raw_, const char* ida_, const char* mask_, int length_ )
        : length( length_ )
        , index( index_ ) 
    {
        memset( raw, 0, sizeof( raw ) );
        memset( ida, 0, sizeof( ida ) );
        memset( hybrid, 0, sizeof( hybrid ) );
        memset( mask, 0, sizeof( mask ) );

        memcpy( raw, raw_, length + 1 );
        strcpy( ida, ida_ );
        strcpy( mask, mask_ );

        std::random_device dev;
        std::uniform_int_distribution<> distrib( 0, 255 );

        // Randomize wildcard
        do{
            wildcard = distrib( dev );
        } while (std::find( raw, raw + length, (char)wildcard ) != raw + length);

        for (int i = 0; i < length; i++)
        {
            if (mask[i] == '?')
                strcat( hybrid, "00" );
            else
            {
                char buf[3] = { 0 };
                sprintf( buf, "%02x", raw[i] & 0xFF );
                strcat( hybrid, buf );
            }

            if (i != length - 1)
                strcat( hybrid, " " );
        }

        for (int i = 0; i < length; i++)
            if (mask[i] == '?')
                raw[i] = wildcard;
    }
};

static std::vector<Pattern*> g_patterns;

static size_t RegPattern( int idx, const char* r, const char* i, const char* m, int l )
{
    g_patterns.push_back( new Pattern( idx, r, i, m, l ) );
    return g_patterns.size();
}

static size_t RegRandomPattern( int idx )
{
    static std::random_device rd;
    std::uniform_int_distribution<> len_distr( 10, 23 );
    std::uniform_int_distribution<> sym_distr( 0, 255 );
    std::uniform_real_distribution<> wcd_distr( 0, 1 );

    int len = len_distr( rd );
    char* pattern = new char[len + 1]();
    char* mask = new char[len + 1]();
    char* ida = new char[len * 3 + 1]();

    for (int i = 0, ofs = 0; i < len; i++)
    {
        // wildcard probability 25%
        bool isWD = wcd_distr( rd ) >= 0.75;
        if (i == 0 || i >= len - 3)
            isWD = false;

        pattern[i] = isWD ? 0 : sym_distr( rd );
        mask[i] = isWD ? '?' : 'x';

        if (isWD)
        {
            sprintf( ida + ofs, "? " );
            ofs += 2;
        }
        else
        {
            sprintf( ida + ofs, "%02X ", pattern[i] & 0xFF );
            ofs += 3;
        }
    }

    g_patterns.push_back( new Pattern( idx, pattern, ida, mask, len ) );

    delete[] ida;
    delete[] pattern;
    delete[] mask;

    return g_patterns.size();
}

#define REG_PATTERN(idx, r, i, m) static size_t pattern##idx = RegPattern(idx, r, i, m, (_countof(r) - 1));
#define REG_RANDOM_PATTERN(idx) static size_t pattern##idx = RegRandomPattern(idx);
#endif // PATTERN_H