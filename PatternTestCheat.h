#ifndef PATTERNTESTCHEAT_H
#define PATTERNTESTCHEAT_H

class PatternTestCheat: public PatternTestBase
{
public:
    virtual ~PatternTestCheat() { }

    virtual bool RunAll( const std::vector<struct PatternScanner*>& scanners, const std::vector<struct Pattern*>& patterns )
    {
        for (auto scanner : scanners)
            RunOne( scanner, patterns.front() );

        return true;
    }

    virtual bool RunOne( PatternScanner* scanner, Pattern* pattern )
    {
        std::cout << scanner->name() << " - ";
        memset( mBase, pattern->raw[0], pattern->length );
        memcpy( mBase + 5, pattern->raw, pattern->length );

        try{
            // see if people don't cheat by skipping entire pattern forward.
            scanner->init( pattern );
            if ((mBase + 5) != scanner->runOne( mBase, mSize )) {
                std::cout << "Failed" << std::endl;
                return false;
            }
            else
                std::cout << "OK" << std::endl;
        }
        catch (...)
        {
            std::cout << "Unhanded exception during execution" << std::endl;
        }

        return true;
    }

    virtual const char* Name()
    {
        return "Testing for pattern length cheating";
    };

private:
};

REG_TEST( PatternTestCheat );

#endif// PATTERNTESTCHEAT_H