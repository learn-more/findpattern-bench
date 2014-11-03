#ifndef PATTERNTESTZEROMEM_H
#define PATTERNTESTZEROMEM_H

class PatternTestZeroMem : public PatternTestBase
{
public:
    virtual ~PatternTestZeroMem() { }

    virtual bool RunOne( PatternScanner* scanner, Pattern* pattern )
    {
        auto len = pattern->length;
        auto patternTarget = mBase + mSize - 0x200;
        memcpy( patternTarget, pattern->raw, len );
        for (int i = 0; i < len; i++){
            if (pattern->mask[i] == '?')
                patternTarget[i] = 0x11;
        }

        scanner->init( pattern );
        mTimer.start();
        for (int n = 0; n < kNumRuns; ++n) {
            if (scanner->runOne( mBase, mSize ) != patternTarget) {
                mTimer.stop();
                std::cout << "Failed to find pattern." << std::endl;
                memset( patternTarget, 0, pattern->length );
                return false;
            }
        }
        mTimer.stop();
        double result = mTimer.milli_hp();
        std::cout << "Finding pattern " << pattern->index << " x " << kNumRuns << " took " << result << " ms." << std::endl;
        memset( patternTarget, 0,len );

        return true;
    }

    virtual const char* Name()
    {
        return "Testing on empty memory";
    };

};

REG_TEST( PatternTestZeroMem );

#endif // PATTERNTESTZEROMEM_H