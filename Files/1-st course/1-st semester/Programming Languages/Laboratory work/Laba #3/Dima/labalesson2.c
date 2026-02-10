#include <stdio.h>
void main ()
{

float value1, value2;
char op;
printf("Bвeдитe ваше выражение. \n");
scanf( "%f%c%f", &value1, &op, &value2);
if (op == '+')
printf ("%.2f\n ", value1 + value2);
else if (op == '-')
printf ("%.2f\n ", value1 - value2);
else if (op == '*')
printf ("%.2f\n ", value1 * value2);
else if (op == '/')
    if (value2 == 0){
        printf("Второе число равно 0. Результат действия равен бесконечно большому числу");
    }
    else
        printf ("%.2f\n ", value1 / value2);
}