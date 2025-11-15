#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[], char* env[]) {
    printf("Количество аргументов командной строки %i \n", argc);
    printf("Аргументы командной строки:\n");
    for (int i = 0; i < argc; i++)
        printf("%s\n", argv[i]);

  
}