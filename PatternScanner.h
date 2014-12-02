#ifndef PATTERNSCANNER_H
#define PATTERNSCANNER_H

struct PatternScanner
{
public:
    virtual void init( Pattern* pattern ) = 0;
    virtual LPVOID runOne( PBYTE baseAddress, DWORD size ) = 0;
    virtual const char* name() const = 0;
};

static std::vector<PatternScanner*> g_scanners;

size_t RegScanner( PatternScanner* b )
{
    g_scanners.push_back( b );
    return g_scanners.size();
}

#define XREG_SCAN2(x,y)	static size_t scanner_##x_##y = RegScanner( new x() );
#define XREG_SCAN(x,y)  XREG_SCAN2(x, y)
#define REG_SCAN(x)     XREG_SCAN(x, __LINE__)

#endif // PATTERNSCANNER_H