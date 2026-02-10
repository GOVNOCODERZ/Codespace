#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* function(double a, double b) {
    if (a > b)
        return ">";
    else if (a < b)
        return "<";
    else
        return "=";
}


int main() {
    int num1, num2;

    printf("Введите первое число: ");
    scanf("%d", &num1);

    printf("Введите второе число: ");
    scanf("%d", &num2);

    printf("%d %s %d", num1, function(num1, num2), num2);

    return 0;
}