#ifndef PATTERNTESTBOUNDS_H
#define PATTERNTESTBOUNDS_H

class PatternTestBounds: public PatternTestBase
{
public:
    virtual ~PatternTestBounds() { }

    virtual bool RunAll( const std::vector<struct PatternScanner*>& scanners, const std::vector<struct Pattern*>& patterns )
    {
        for (auto scanner : scanners)
            RunOne( scanner, patterns.front() );

        return true;
    }

    virtual bool RunOne( PatternScanner* scanner, Pattern* pattern )
    {
        std::cout << scanner->name() << " - ";
        __try 
        {
            scanner->init( pattern );
            scanner->runOne( mBase, mSize );		// see if it stops nicely at the end.
            *(mBase + mSize - 1) = pattern->raw[0];
            scanner->runOne( mBase, mSize );		// see if it stops nicely at the end.
            *(mBase + mSize - 1) = 0;
            std::cout << "OK" << std::endl;
            return true;
        }
        __except (1)
        {
            std::cout << "ran outside the area" << std::endl;
            return false;
        }

        return true;
    }

    virtual const char* Name()
    {
        return "Testing for invalid memory access";
    };

private:
};

REG_TEST( PatternTestBounds );

#endif // PATTERNTESTBOUNDS_H