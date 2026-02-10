#include <fstream>
#include <iostream>
using namespace std;

int neg_num_count (char*file)
{
    int count = 0;
    int number = 0;
    
    ifstream fin;
    fin.open(file, ios::in);
    
    if(!fin)
    {
        return-1;
    }

    while (fin>>number)
        if(number < 0)
            count++;

    fin.close();

    return count;
}

int main()
{
    int c = neg_num_count("lab2(3).txt");
    if(c == -1)
        cout <<"\nНет потока!"<< c << endl;
    else
        cout <<"\nКоличество отрицательных чисел в файле = "<< c << endl;
    return 0;
}