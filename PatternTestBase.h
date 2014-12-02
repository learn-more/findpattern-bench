#ifndef PATTERNTEST_H
#define PATTERNTEST_H

class PatternTestBase
{
public:
    virtual ~PatternTestBase() { }

    virtual void SetMemoryRegion( PBYTE start, size_t size )
    {
        mBase = start;
        mSize = size;
    }

    virtual void Init()
    {
        memset( mBase, 0, mSize );
    }

    virtual bool RunAll( const std::vector<struct PatternScanner*>& scanners, const std::vector<struct Pattern*>& patterns )
    {
        bool success = true;
        for (auto scanner : scanners)
            if (!RunPatterns( scanner, patterns ))
                success = false;

        return success;
    }

    virtual bool RunPatterns( struct PatternScanner* scanner, const std::vector<struct Pattern*>& patterns )
    {
        bool success = true;
        std::cout << "========" << scanner->name() << "========" << std::endl;
        for (auto pattern : patterns)
        {
            try{
                Init();
                if (!RunOne( scanner, pattern ))
                    success = false;
            }
            catch (...)
            {
                std::cout << "Exception during execution" << std::endl;
                success = false;
            }
        }

        return success;
    }

    virtual bool RunOne( struct PatternScanner* scanner, struct Pattern* pattern ) = 0;
    virtual const char* Name() = 0;

protected:
    Timer mTimer;
    PBYTE mBase;
    size_t mSize;
};

static std::vector<PatternTestBase*> g_tests;

size_t RegTest( PatternTestBase* test )
{
    g_tests.push_back( test );
    return g_tests.size();
}

#define XREG_TEST2(x, y) static size_t test_case_##y = RegTest( new x() );
#define XREG_TEST(x,y)	 XREG_TEST2(x, y)
#define REG_TEST(x)		 XREG_TEST(x, __LINE__)

#endif // PATTERNTEST_H