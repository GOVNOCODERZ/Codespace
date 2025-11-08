#include <stdio.h>
int main ()
{

char с;
printf( "Bвeдитe одиночный символ:\n" );
scanf( "%c" , &с);
if ((с >='a' && с<= 'z' ) || (с>= 'A' && с<= 'Z' ))
printf("Этo символ алфавита.\n");
else if (с>= '0' && с<= '9')
printf("Этo цифра. \n ");
else
printf("Этo специальный символ.\n");
return 0;
}