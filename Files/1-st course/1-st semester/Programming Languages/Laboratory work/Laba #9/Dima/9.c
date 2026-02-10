#include <stdio.h>

int main(int argc, char* argv[], char* env[]){

    //printf("Количество аргументов командной строки %i \n", argc);

 //   printf("Агрументы командной строки:\n");
    for (int i = 0; i < argc; i++)
        printf("%s\n", argv[i]);


    
return 0;
}