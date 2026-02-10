#include <stdio.h>

void main(){
    for (unsigned char ch = 0; ch < 255; ch++){
    printf("Значение ASCII для %c - %i.\n", ch, ch);
    //if (ch == 254){
        printf("Значение ASCII для %c - %i.\n", ch+1, ch+1);
    }
    }
}
