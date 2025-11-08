#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double calculate(double num1, double num2, char op) {
    switch (op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            return num1 / num2;
    }
}

int main(int argc, char *argv[], char *envp[]){
    double num1 = 0, num2 = 0;
    char op;

    num1 = atof(argv[1]);
    op = argv[2][0];
    num2 = atof(argv[3]);

    for (int i = 0; i < argc; i++)
        printf("%s\n", argv[i]);

    double result = calculate(num1, num2, op);

    printf(" = %f\n", result);

}