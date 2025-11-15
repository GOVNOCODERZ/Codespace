#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void rpEO(int *a, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (a[i] % 2 == 0 && a[i + 1] % 2 != 0) {
            int temp = a[i];
            a[i] = a[i + 1];
            a[i + 1] = temp;
            i++;
        }
    }
}

void printArr(int *a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}   

int main() {
    int N;

    srand(time(NULL));

    printf("Введите размер массива: ");
    scanf("%d", &N);

    int *a = (int *)malloc(N * sizeof(int));
    if (a == NULL) {
        printf("Ошибка распределения памяти!.\n");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        a[i] = rand() % 50;
    }


    printf("Исходный массив: ");
    printArr(a, N);

    rpEO(a, N);

    printf("Измененный массив: ");
    printArr(a, N);

    free(a);

    return 0;
}