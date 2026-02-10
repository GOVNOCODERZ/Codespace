#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(){
    int N, i;
    printf("\nВведите длину массива: ");
    scanf("%d", &N);

    char LIST[N];
    srand(time(NULL));
    int m;
    for (i = 0; i < N; i++)
        m = rand();
        LIST[i] = m;
    char *address = &LIST[0];
    for (i = 0; i < N; i++)
        printf("\nЭлемент массива = %d, адрес элемента = %d", *(address + N - i), address + N - i);

    return 0;
}