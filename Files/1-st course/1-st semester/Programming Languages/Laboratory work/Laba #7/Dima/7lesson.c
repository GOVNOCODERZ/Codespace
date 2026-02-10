#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int N;
const int min = 1;
const int max = 20;
double a[100];


int main(){

    srand((unsigned)time(0));

    printf("Введите длину массива:\n"); scanf("%i", &N);
    for (int i = 0; i < N; i++){
        void* address = a + i; 
        int u = (double)rand() / (RAND_MAX + 1) * (max - min) + min;
        a[i] = u;
        int value = *(a + i);  
        printf("a[%i]: %lf, adress = %d\n",i, a[i], address);
    }
    for (int i = 0; i<N; i++){
        int value = *(a+N-i-1);
        printf("значение a[%i] = %d\n",i, value);
    }
    return 0;
}
//Второй массив с выведением ячейки через указатель
//Вывести вверх ногами