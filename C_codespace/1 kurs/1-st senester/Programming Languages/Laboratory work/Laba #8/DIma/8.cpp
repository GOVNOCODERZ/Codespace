#include <stdio.h>
#include <malloc.h>
int main(){
    int rows = 2;
    int cols = 5;
    int **rooms;

    rooms = new int *[rows];
    for (int i = 0; i < rows; i++){
        rooms[i] = new int[cols];

}
    for(int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            rooms[i][j] = rand()%10;
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            printf("%4i", rooms[i][j]);
        }
        puts("");
}
    for (int i = 0; i < rows; i++){
        delete[] rooms[i];
}
return 0;
}