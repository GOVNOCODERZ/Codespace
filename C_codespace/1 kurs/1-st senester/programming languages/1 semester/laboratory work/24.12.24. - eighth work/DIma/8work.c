#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main() {
    int N;

    printf("Введите размер массива: ");
    scanf("%d", &N);

    int *X = (int *)malloc(N * sizeof(int));
    if (X == NULL) {
        printf("Ошибка распределения памяти!.\n");
        return 1;
    }

    printf("Введите элементы массива:\n");
    for (int i = 0; i < N; i++) {
        scanf("%d", &X[i]);
    }

    int maxOdd = INT_MIN;
    int minEven = INT_MAX;
    int Odd = 0;
    int Even = 0; 

    for (int i = 0; i < N; i++) {
        if (X[i] > 0) 
        {
            if (X[i] % 2 == 0) 
            {
                Even = 1;
                if (X[i] < minEven) {
                    minEven = X[i];
                }
            }
            else 
            {
                Odd = 1;
                if (X[i] > maxOdd) 
                {
                    maxOdd = X[i];
                }
            }
        }
    }


    if (Odd) {
        printf("Максимальный положительный нечетный элемент: %d\n", maxOdd);
    } 

    else {
        printf("Нет положительных нечетных элементов.\n");
    }

    if (Even) {
        printf("Минимальный положительный четный элемент: %d\n", minEven);
    } 

    else {
        printf("Нет положительных четных элементов.\n");
    }

    free(X);

    return 0;
}