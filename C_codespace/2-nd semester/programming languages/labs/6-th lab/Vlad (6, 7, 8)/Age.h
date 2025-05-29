#ifndef AGE_H
#define AGE_H
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

class Age {
    private:
        string name;
        int years; 
        int months;
        int days;
        static int objectCount;

    public:
        void input();
        void output();
        int calculateDays();
        
        Age(): 
            name("Object #" + to_string(rand() % 100)), 
            years(rand() % 100), 
            months(rand() % 12), 
            days(rand() % 30){++objectCount;}

        Age(string myName, int myYears, int myMonths, int myDays): 
            name(myName), 
            years(myYears), 
            months(myMonths), 
            days(myDays){++objectCount;}

        Age(const Age&A):
            name(A.name), 
            years(A.years), 
            months(A.months), 
            days(A.days){++objectCount;}

        ~Age(){--objectCount;}

        Age(int totalDays):
            name("test_name"), 
            years(totalDays / 365), 
            months((totalDays % 365) / 30), 
            days((totalDays % 365) % 30){++objectCount;}

            
        Age& operator=(const Age& other);
        Age& operator*=(int multiplier);
        bool operator<(Age& other);


        friend std::istream& operator>>(std::istream&, Age&);
        friend std::ostream& operator<<(std::ostream&, const Age&);

        static int getObjectsCount(){ 
            return objectCount;
        }
    };
#endif