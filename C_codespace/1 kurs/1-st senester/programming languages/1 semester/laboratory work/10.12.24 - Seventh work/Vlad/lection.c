#include <stdio.h>

int x = 5, y, *px = &x;

int main(){
    
    y = *px + 2;
    printf("y = %i значение указателя = %i\n", y, px);

    y = *px++;
    printf("y = %i значение указателя = %i\n", y, px);

    px = &x;
    y = (*px)++;

    printf("y = %i значение указателя = %i. Значение, адресуемое указателем *px = %i\n", y, px, *px);

    y = ++ *px;
    printf("y = %i значение указателя = %i\n", y, px);

    return 0;
}