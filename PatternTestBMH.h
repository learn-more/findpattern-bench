#ifndef PATTERNTESTBMH_H
#define PATTERNTESTBMH_H

class PatternTestBMH: public PatternTestBase
{
public:
    virtual ~PatternTestBMH() { }

    virtual bool RunAll( const std::vector<struct PatternScanner*>& scanners, const std::vector<struct Pattern*>& patterns )
    {
        for (auto scanner : scanners)
        {
            bool failed = false;

            std::cout << scanner->name() << " - ";
            for (auto pattern : patterns)
                if (!RunOne( scanner, pattern )) {
                    failed = true;
                    std::cout << "Failed on pattern #" << pattern->index << std::endl;
                    break;
                }

            if (!failed)
                std::cout << "OK" << std::endl;
        }

        return true;
    }

    virtual bool RunOne( PatternScanner* scanner, Pattern* pattern )
    {
        size_t last = pattern->length - 1;
        size_t idx = 0;

        // Get last wildcard position
        for (idx = last; idx > 0 && pattern->mask[idx] != '?'; --idx);
        size_t diff = last - idx;

        // No wildcards, can't fail
        if (diff == last)
            return true;
        // Last symbol is a wildcard, handle separately
        else if (diff == 0)
            diff = 1;

        memset( mBase, 0, pattern->length * 2 );
        memcpy( mBase + diff, pattern->raw, pattern->length );
        mBase[pattern->length - 1] = pattern->raw[1];

        try{
            // see if people don't cheat by skipping too many bytes
            scanner->init( pattern );
            if ((mBase + diff) != scanner->runOne( mBase, mSize )) {
                return false;
            }   
        }
        catch (...) {
            std::cout << "Unhanded exception during execution" << std::endl;
            return false;
        }

        return true;
    }

    virtual const char* Name()
    {
        return "Inverse pattern length cheating test";
    };

private:
};

REG_TEST( PatternTestBMH );

#endif// PATTERNTESTCHEAT_H