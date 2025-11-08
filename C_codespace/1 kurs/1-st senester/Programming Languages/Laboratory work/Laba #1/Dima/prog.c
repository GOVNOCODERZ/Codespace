
#include <stdio.h>
#include <math.h>

void main() {
    double a,b,c,p,s;
    printf("\na="); scanf("%lf",&a);
    printf("\nb="); scanf("%lf",&b);
    printf("\nc="); scanf("%lf",&c);
    p = (a+b+c)/2;
    s = sqrt(p*(p-a)*(p-b)*(p-c));
    printf("\nПлощадь треугольника=%lf",s);
}