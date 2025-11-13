#include <stdio.h>
#include <stdlib.h>

void bubblesort(int* mass, int size) {
    for (int k = 0; k < size; k++) {
        for (int l = size - 1-k; l > k; l--) {
            if (mass[l] > mass[l-1]) {
                int temp = mass[l - 1];
                mass[l - 1] = mass[l];
                mass[l] = temp;
            }
        }
    }
}
++

int main() {
    int N, i;
    int k1 = 0;
    int k2 = 0;
    int k3 = 0;

    printf("Укажите количество элементов вашего массива: ");
    scanf("%d", &N);
    int X[N];

    printf("Введите %d чисел (через пробел): ", N);
    for(i = 0; i < N; i++)
        scanf("%d", &X[i]);
    bubblesort(X, N);
    printf("Упорядоченный по убыванию массив: ");
    for (i = 0; i < N; i++)
        {
            printf("%d ", X[i]);
            if (X[i] > 0)
                k1++;
            else if (X[i] < 0)
                k2++;
            else
                k3++;
        };
    printf("\nКоличество положительных элементов - %d\nКоличество отрицательных элементов - %d\nКоличество нулевых элементов - %d", k1, k2, k3);
    
    return 0;
}