#ifndef SUPERDOC1234_H
#define SUPERDOC1234_H

byte* FindPattern( byte* pBaseAddress, byte* pbMask, const char* pszMask, size_t nLength )
{
    auto DataCompare = []( const auto* pData, const auto* mask, const auto* cmask, auto chLast, size_t iEnd ) -> bool {
        if( pData[ iEnd ] != chLast ) return false;
        for( ; *cmask; ++cmask , ++pData , ++mask ) {
            if( *cmask == 'x' && *pData != *mask ) {
                return false;
            }
        }

        return true;
    };

    auto iEnd = strlen( pszMask ) - 1;
    auto chLast = pbMask[ iEnd ];

    auto* pEnd = pBaseAddress + nLength - strlen( pszMask );
    for( ; pBaseAddress < pEnd; ++pBaseAddress ) {
        if( DataCompare( pBaseAddress, pbMask, pszMask, chLast, iEnd ) ) {
            return pBaseAddress;
        }
    }

    return nullptr;
}

struct SUPERDOC : public BenchBase
{
    virtual void init( Tests test )
    {
        switch( test ) {
        case Tests::First:
            pbPattern = reinterpret_cast< byte* >( "\x45\x43\x45\x55\x33\x9a\xfa\x00\x00\x00\x00\x45\x68\x21" );
            pszMask = "xxxxxxx????xxx";
            break;
        case Tests::Second:
            pbPattern = reinterpret_cast< byte* >( "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xaa\x00\x00\x00\x00\x45\x68\x21" );
            pszMask = "xxxxxxxxxxx????xxx";
            break;
        default:
            break;
        }
    }

    virtual void* runOne( PBYTE baseAddress, DWORD size )
    {
        return FindPattern( baseAddress, pbPattern, pszMask, size );
    }

    virtual const char* name() const
    {
        return "superdoc1234";
    }

    byte* pbPattern; // = reinterpret_cast<PCHAR>("");
    char* pszMask; // = reinterpret_cast<PCHAR>("");
};

REGISTER( SUPERDOC );

#endif // SUPERDOC1234_H


