
#include <stdio.h>
#include <math.h>

void main() {
    double a, b, y, z;
    printf("\na="); scanf("%lf",&a);
    printf("\nb="); scanf("%lf",&b);
    y = (sin(a) + cos(2*b-a))/(cos(a)-sin(2*b-a));
    z = (1+sin(2*b))/cos(2*b);
    printf("\ny=%lf",y);
    printf("\nz=%lf",z);
}