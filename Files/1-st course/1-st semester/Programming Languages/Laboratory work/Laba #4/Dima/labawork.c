#include <stdio.h>
#include <math.h>

void main(){
    int x1; int x2; int flag = 0;
    printf("Введите первый элемент последовательности:\n"); 
    scanf("%i", &x1);
do{
        x2 = x1;
        printf("Введите следующий элемент последовательности: \n");
        scanf("%i", &x1);
        if (x1>x2){
            flag++;
        }
}while (x1 != 0);
printf("flag = %i\n", flag);
if (flag != 0){
    printf("Последовательность не убывающая");
}
else{
    printf("Последовательность строго убывающая");
}
}