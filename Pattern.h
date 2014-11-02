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

        // Randomize wildcard
        std::random_device dev;
        std::uniform_int_distribution<> distrib( 0, 255 );
        
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

size_t RegPattern( int idx, const char* r, const char* i, const char* m, int l )
{
    g_patterns.push_back( new Pattern( idx, r, i, m, l ) );
    return g_patterns.size();
}

#define REG_PATTERN(idx, r, i, m) static size_t pattern##idx = RegPattern(idx, r, i, m, (_countof(r) - 1));

#endif // PATTERN_H