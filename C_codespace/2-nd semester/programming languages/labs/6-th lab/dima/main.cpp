#include "Angle.h"
#include <windows.h>

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    Angle angle;
    
    angle.input();
    angle.display();
    
    int totalSeconds = angle.toSeconds();
    std::cout << "Общее количество секунд: " << totalSeconds << "\n";
    
    return 0;
}