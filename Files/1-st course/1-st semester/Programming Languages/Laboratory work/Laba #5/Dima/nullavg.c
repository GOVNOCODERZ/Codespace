#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int mass[10];
unsigned int umn = 1;
float aver = 0;
int cnt = 0;
void main(void)
{
srand((unsigned)time(0));

for(int i = 0; i < 10; ++i)
    mass[i] = rand()%20; srand(2);
printf("Элементы массива: ");

for(int i = 0; i < 10; ++i)
{ 
    printf("%d ", mass[i]);
    aver += mass[i];
    if (mass[i] != 0)
        umn = umn * mass[i];
}

for(int i = 0; i < 10; ++i)
{
if (mass[i]>(aver/10))
cnt+=1;
}
printf("\nПроизведение ненулевых элементов: %i", umn);
printf("\nСреднее арифметическое: %f",aver/10);
printf("\nКоличество элементов массива, первышающих среднее\
арифметическое: %i", cnt);
}