#include <stdio.h>
#include <windows.h>
#include <math.h>

int main(int argc, char** argv)
{
    LARGE_INTEGER timerFrequency, timerStart, timerStop;
    QueryPerformanceFrequency(&timerFrequency);
    QueryPerformanceCounter(&timerStart);
    printf("\nSize = %i\n", sizeof(size_t));

    unsigned long long s = 0, i;
    const unsigned long N = 3e+9;

    for (i = 1; i <= N; i+=10){
        s+= N/i;
        s+= N/(i+1);
        s+= N/(i+2);
        s+= N/(i+3);
        s+= N/(i+4);
        s+= N/(i+5);
        s+= N/(i+6);
        s+= N/(i+7);
        s+= N/(i+8);
        s+= N/(i+9);
    }


    printf("\ns = %lld\n", s);
    
    QueryPerformanceCounter(&timerStop);
    double t = (double)(timerStop.QuadPart - timerStart.QuadPart) / timerFrequency.QuadPart;
    printf("Time is %lf seconds.\n",t);
    return 0;
}