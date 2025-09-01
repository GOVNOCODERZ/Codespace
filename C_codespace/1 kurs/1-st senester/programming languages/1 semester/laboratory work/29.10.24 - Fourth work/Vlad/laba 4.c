#include <stdio.h>

int main(){
    int N;
    printf("\nВведите число N: "); scanf("%d", &N);
    double summ = 0;
    double diff = 0;
    int i = 0;
    do
    {
        double number = 0;
        printf("\nВведите число: "); scanf("%lf", &number);
        if (number > 0)
            summ += number;
        else if (number < 0)
            diff -= number;
        i++;
    }while(i < N);
    
    printf("\nСумма = %lf\nРазность = %lf", summ, diff);
    return 0;
}