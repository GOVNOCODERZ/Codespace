#include <stdio.h>

#define TWO 2
#define FOUR TWO * TWO
#define PX printf("X равно %i.\n", x)
#define FMT "X равно %i.\n"
#define SQUARE(X) (X) * (X)

int main()
{
    int x = TWO;
    PX;
    x = FOUR;
    printf(FMT, x);
    x = SQUARE(3-3);
    PX;
    return 0;
}