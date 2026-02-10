#include <iostream>
#include <fstream>
#include <iomanip>

int main(){
    std::ofstream fin;
    fin.open(r"C:\Users\luvidmi\Desktop\git\Codespace\C_codespace\2-nd semester\programming languages\labs\2-nd lab\Dima\govnotext.txt"); 
    if(fin.is_open())
    {
        fin<<"124"<<std::endl;
    }
    fin.close();
    std::cout<<"File has been written"<<std::endl;
    
}