#include <fstream>
#include <iostream>
using namespace std;
int change(char*filenamein, char*filenameout){
    ifstream fin;
    ofstream fout;
    fin.open(filenamein, ios::in);
    if (!fin){
        cout<<"\n Нет исходного файла";
        return -1;
    }
    fout.open(filenameout);
    if (!fout){
        cout<<"Ошибка при открытии файла";
        return -1;
    }

    char letter;
    while (fin.get(letter)){
        if (isupper(letter)){
            letter = tolower(letter);
        }
        else if (islower(letter)){
            letter = toupper(letter);
        }
        fout.put(letter);
    }
    fin.close();
    fout.close(); 
    return 0;
}

int main(){
    int result = change("C://Users//luvidmi//Desktop//git//Codespace//C_codespace//programming languages//2 semester//laboratory work//06.03.25//dima//letters(in).txt", \
        "C://Users//luvidmi//Desktop//git//Codespace//C_codespace//programming languages//2 semester//laboratory work//06.03.25//dima//letters(out).txt");
    if (result == -1){
        cout<<"Нет потоков";
    }

    cout<<"Замена регистра была произведена. Изменения сохранены в файле 'letters.txt'";
    return 0;
}
