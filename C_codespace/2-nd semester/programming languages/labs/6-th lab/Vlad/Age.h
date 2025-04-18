#ifndef AGE_H
#define AGE_H
#include <cstdlib>
#include <ctime>

class Age {
    private:
        string name;
        int years;  
        int months; 
        int days;

    public:
        void input();
        void output();
        int calculateDays();
        
        Age(): 
            name("Object #" + to_string(rand() % 100)), 
            years(rand() % 100), 
            months(rand() % 12), 
            days(rand() % 30){}

        Age(string myName, int myYears, int myMonths, int myDays): 
            name(myName), 
            years(myYears), 
            months(myMonths), 
            days(myDays){}

        Age(const Age&A):
            name(A.name), 
            years(A.years), 
            months(A.months), 
            days(A.days){}

        ~Age(){}

        Age(int totalDays):
            name("test_name"), 
            years(totalDays / 365), 
            months((totalDays % 365) / 30), 
            days((totalDays % 365) % 30){}
    };
#endif