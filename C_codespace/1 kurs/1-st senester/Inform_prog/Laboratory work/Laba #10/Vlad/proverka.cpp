#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
using namespace std;
int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    ifstream in("text.txt");
    string str;
    getline(in,str);
    cout<<str<<endl;
    return 0;
}