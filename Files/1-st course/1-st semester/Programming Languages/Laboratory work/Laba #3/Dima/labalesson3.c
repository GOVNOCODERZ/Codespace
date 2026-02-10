#include <stdio.h>
void main(){
    int x;
    char ch;
    printf("Введите число: ");
    scanf("%i", &x);

    switch (x)
    {
        case 1: printf("Введено число 1\n"); break;
        case 2: printf("Введено число 2\n"); break;
        default: printf("Введено другое число\n");
    }
    printf("Введите символ: ");
    scanf("%c", &ch);

    switch (ch)
    {
        case 'a': printf("Введен смвол a\n"); break;
        case 'b': printf("Введен символ b\n"); break;
        default: printf("Введен другой символ\n");
    }
}