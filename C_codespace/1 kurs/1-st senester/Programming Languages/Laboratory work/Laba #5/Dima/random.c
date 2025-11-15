#include <stdlib.h>
#include <stdio.h>
#include <time.h>
void main ()
{
const int max = 100;
const int min = 50;
const int N = 20;
srand (( unsigned )time (0));
for (int i = 1; i <= N; i++) {
int u = ( double ) rand () / (RAND_MAX + 1) * (max - min) + min;
printf ( "%i . Псевдослучайное число в диапазоне [%i , %i ] = %i \n ", i, min, max, u);
}
}