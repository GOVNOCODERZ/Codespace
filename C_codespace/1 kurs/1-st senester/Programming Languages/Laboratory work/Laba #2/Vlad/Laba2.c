#include <stdio.h>
#include <math.h>
#include <C:\Codespace\C_codespace\programming languages\1 semester\laboratory work\01.10.24 - Second work\Vlad\const.h>

int main(){
    double N;
    double Time_sec, Time_min, Floor;
    printf("\nВведите высоту: "); scanf("%lf",&N);
    Time_sec = N / SPEED;
    Time_min = Time_sec / 60;
    Floor = round(N / FLOORX);
    printf("Лифт поднимется на высоту %.0lf метров за %.0lf секунд или за %.2lf минуты.\n", N, Time_sec, Time_min);
    printf("Это - %.0lf этаж.\n", Floor);
    return 0;
}