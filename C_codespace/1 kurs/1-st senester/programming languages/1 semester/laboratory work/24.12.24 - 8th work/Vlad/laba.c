#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int main(){
    int N, i, j, m, k;
    k = m = 0;

    printf("\nВведите длину массива: ");
    scanf("%d", &N);

    double *X = (double *)malloc(N * sizeof(double));

    printf("Введите элементы массива:\n");
    for (int i = 0; i < N; i++) {
        scanf("%lf", &X[i]);
    }

    for (i = 0; i < N; i++)
        {
            if (X[i] > 0)
                k++;
            else if (X[i] < 0)
                m++;
        };

    double *Y = (double *)malloc(k * sizeof(double));
    double *Z = (double *)malloc(m * sizeof(double));
    int k1 = 0;
    int m1 = 0;
    int x;
    for (i = 0; i < N; i++)
        {
            if (X[i] > 0)
            {
                Y[k1] = X[i];
                k1++;
            }

            else if (X[i] < 0)
            {
                Z[m1] = X[i];
                m1++;
            }
        };

    for (i = 0; i < k; i++)
        printf("\n%lf", Y[i]);

    printf("\n----------------------------\n");

    for (i = 0; i < m; i++)
        printf("\n%lf", Z[i]);
        
    return 0;
}