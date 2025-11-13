#include <stdio.h>
#include <math.h>
void main(){
    int x;
    int primer;
    printf("Введите натуральное число x, 100<=x<=10000: ");
    scanf("%d",&x);
    if (x%2==0){
        primer = pow((x-2),2);
        printf("Число x чётное, f(x)=%d", primer);}
    else{
        primer = pow((2-x),2);
        printf("Число x нечётное, f(x)=%d", primer);}



}