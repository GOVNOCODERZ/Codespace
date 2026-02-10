#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>
#include <new.h>

int rows = 2;
int cols = 5;

int **rooms;

int main(){
    rooms = new int *[rows];
    for (int i = 0; i < rows; i++) {
        rooms[i] = new int[cols];
    }

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < rows; j++)
            rooms[i][j] = rand()%10;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j = cols; j++) {
            printf("%4i", rooms[i][j]);
        }
        puts("");
    }

    for (int i = 0; i < rows; i++) {
        delete[] rooms[i];
    }
    delete[] rooms;

    return 0;
}