#include <fstream>
#include <iostream>
using namespace std;

const int MAX_STRING_SIZE = 100;

int file_filling (char*file1, char*file2)
{
    char string[MAX_STRING_SIZE];
    
    ifstream fin;
    fin.open(file1, ios::in);

    ofstream fout;
    fout.open(file2, ios::out|ios::app);
    
    if(!fin)
    {
        cout <<"\nФайл для чтения не найден!"<< endl;
        return 1;
    }

    if(!fout)
    {
        cout <<"\nФайл для записи не найден!"<< endl;
        return 1;
    }

    while (fin.getline(string, MAX_STRING_SIZE))
        fout<<string<<endl;

    fin.close();
    fout.close();

    return 0;
}

int main()
{   
    cout <<"\nНачинаем перепись из первого файла."<< endl;
    if(file_filling("text_1.txt", "text_3.txt") == 1)
        cout <<"\nОШИБКА: Перепись из первого файла не удалась!"<< endl;
    else
        cout <<"\nПерепись из первого файла успешна!"<< endl;
    
    cout <<"\nНачинаем перепись из второго файла."<< endl;
    if(file_filling("text_2.txt", "text_3.txt") == 1)
        cout <<"\nОШИБКА: Перепись из второго файла не удалась!"<< endl;
    else
        cout <<"\nПерепись из второго файла успешна!"<< endl;

    return 0;
}