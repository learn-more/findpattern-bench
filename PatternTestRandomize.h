#ifndef PATTERNTESTRANDOMIZE_H
#define PATTERNTESTRANDOMIZE_H

class PatternTestRandomize : public PatternTestBase
{
public:
    virtual ~PatternTestRandomize() { }

    virtual void Init()
    {
        // Fill with random data
        for (DWORD i = 0; i < mSize; i += 4 * sizeof( size_t ))
            *(size_t*)(mBase + i) = distrib( engine );
    }

    virtual bool RunOne( PatternScanner* scanner, Pattern* pattern )
    {
        auto len = pattern->length;
        std::uniform_int_distribution<> patternLen_distrib( 1, len - 3 );
        std::uniform_int_distribution<> skip_distrib( len + 100, len + 800 );

        // Add random pattern chunks to the buffer
        for (PBYTE iter = mBase; iter < mBase + mSize - (len + 800); iter += skip_distrib( engine )){
            bool reverse = ((uintptr_t)iter & 1) == 1;
            int chunkLen = patternLen_distrib( engine );

            // Copy last bytes
            if (reverse){
                int ofst = len - chunkLen;
                memset( iter, 0x11, ofst );
                memcpy( iter + ofst, pattern->raw + ofst, chunkLen );
            }
            // Copy first bytes
            else {
                memcpy( iter, pattern->raw, chunkLen );
                memset( iter + chunkLen, 0x11, len - chunkLen );
            }
        }

        auto patternTarget = mBase + mSize - 0x150;
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
        return "Testing on randomized data";
    };

private:
    std::mt19937 engine;
    std::uniform_int_distribution<size_t> distrib;
};

REG_TEST( PatternTestRandomize );

#endif // PATTERNTESTRANDOMIZE_H