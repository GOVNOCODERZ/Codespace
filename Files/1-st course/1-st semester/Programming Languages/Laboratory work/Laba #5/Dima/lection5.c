#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int room[2][10] = { {102, 107, 109, 112, 115, 116, 123, 125, 127, 130},
 {12,0,23,12,20,15,16,23,12,15} };

void main(void) {
    const int min = 1;
    const int max = 5;
    int i, j, flag=2, num;

    srand((unsigned)time(0));

    for (j = 0; j<10; j++) 
    {
        int u = (double)rand() / (RAND_MAX + 1) * (max - min) + min;
        room[1][j] = u;
    }
    puts("Вместимость всех комнат гостиницы:");

    for (j = 0; j<10; j++)
        printf("Комната #%i рассчитана на %i мест\n", room[0][j], room[1][j]);

    puts("Введите минимальное необходимое количество мест");
    scanf("%i", &num);

    for (j = 0; j < 10; j++)
        if (room[1][j] >= num)
        {
            flag = 1;
            printf("Комната #%i рассчитана на %i мест\n", room[0][j], room[1][j]);
        }
    if (flag == 2)
        puts("Комнат с таким количеством мест нет");
}
