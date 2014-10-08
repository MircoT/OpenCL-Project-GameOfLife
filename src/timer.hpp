#include <functional>
#if defined(_MSC_VER)
    #include <windows.h>
#else
#   include <sys/time.h>
#endif

namespace Timer
{
    #if defined(_MSC_VER)
        typedef __int64    bit64;
        typedef __int64    byte16;
    #else
        typedef long long  bit64;
        typedef long long  byte16;
    #endif
	
	inline bit64 getTimeTicks()
	{
		bit64 val;
        #if !defined(_MSC_VER)
            timeval timeVal;
        
            // http://linux.die.net/man/2/gettimeofday
            gettimeofday( &timeVal, NULL );
        
            val = (bit64)timeVal.tv_sec * (1000*1000) + (bit64)timeVal.tv_usec;
        #else
            QueryPerformanceCounter( (LARGE_INTEGER *)&val );
        #endif
		return val;
	}
	//time
	inline double getTime()
	{
		static double coe;
        #if !defined(_MSC_VER)
            coe = 1.0 / 1000.0;
        #else
            static bit64 freq;
        
            if ( freq == 0 )
            {
                QueryPerformanceFrequency( (LARGE_INTEGER *)&freq );
                coe = 1000.0 / freq;
            }
        #endif
        
		return getTimeTicks() * coe;
	}
	//execute task
	inline double timeExecute(std::function<void(void)> function)
	{
		bit64 start=getTime();
		function();
		return getTime()-start;
	}
}
