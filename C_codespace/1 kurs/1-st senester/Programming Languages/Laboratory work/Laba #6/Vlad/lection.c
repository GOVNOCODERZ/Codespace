#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main(void) {
    int age;
    char name[10000], str[10000];
    puts("Введите Ваше имя: ");
    scanf("%s", name);
    printf("Введите Ваш возраст: ");
    scanf("%i", &age);
    sprintf(str, "Здраствуйте, %s. Ваш возраст - %i лет.", name, age);
    puts(str);
}