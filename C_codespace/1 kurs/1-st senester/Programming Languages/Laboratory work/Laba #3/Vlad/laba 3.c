#include <stdio.h>

int main (){
    int newspaper = 15;
    int magazine = 90;
    char choice;
    int summa;
    
    printf("Что вы хотите купить, газету за 15 рублей (1) или журнал за 90 (2)?\n");
    scanf("%c", &choice);
    printf("Какую сумму денег вы готовы потратить на покупку?\n");
    scanf("%d", &summa);

    switch (choice)
    {
        case '1':
            if (summa < newspaper)
                printf("Недостаточно денег для покупки газеты.\n");
            else
                printf("Вы приобрели газету. Ваша сдача - %d.\n", summa - newspaper);
            ;
        case '2':
            if (summa < magazine)
                printf("Недостаточно денег для покупки журнала.\n");
            else
                printf("Вы приобрели журнал. Ваша сдача - %d.\n", summa - magazine);
            break;                         
    };
    return 0;
}