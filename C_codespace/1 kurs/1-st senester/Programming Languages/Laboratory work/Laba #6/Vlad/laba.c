#include <stdio.h>
#include <string.h>
#define SIZE 100 
#include <locale.h>

int main(){
    setlocale(LC_ALL, "Rus");
    char text[SIZE];
    char word[SIZE];
    char* strpoiner = text;
    int count = 0;

    puts("\nВведите строку текста: ");
    gets(text);
    puts("\nВведите слово: ");
    gets(word);

    while ((strpoiner = strstr(strpoiner, word)) != NULL)
    {
            count++;
            strpoiner++;
    }

    printf("Указанное слово встречается в тексте %d раз.", count);
    getchar();
    return 0;
}

/*
#include <string.h>
#include <stdio.h> 
#define SIZE 100                     
 
int main()
{
    char  s[SIZE];                  
    char  substr[SIZE];        
    int   n = 0;                 
    char* temp = s;                  
    
    puts("Vvedite stroku: \n");     
    gets(s);                        
    puts("\nVvedite slovo: \n"); 
    gets(substr);             
   
  
    while((temp = strstr(temp, substr)) != NULL) 
    {
                                    
        n++;                        
        temp++;                      
                                     
    }
    printf("\nEto slovo vstrechaetsya %d raz(a)!", n);
    getchar();
    return 0;
}
*/