#ifndef PAYMENT_H
#define PAYMENT_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Payment {
protected:
    string date;
    float amount;

public:
    Payment();
    Payment(const string& date, float amount);
    Payment(const Payment& other);
    virtual ~Payment();

    virtual Payment* getCopy() const = 0;

    virtual string myName() const = 0;

    virtual void input(istream& is);
    virtual void output(ostream& os) const;

    // Геттеры
    string getDate() const { return date; }
    float getAmount() const { return amount; }

    // Виртуальные операторы ввода/вывода
    friend istream& operator>>(istream& is, Payment& p);
    friend ostream& operator<<(ostream& os, const Payment& p);
};

class BankTransfer : public Payment {
private:
    string bank;

public:
    BankTransfer();
    BankTransfer(const string& date, float amount, const string& bank);
    BankTransfer(const BankTransfer& other);
    virtual ~BankTransfer();

    virtual Payment* getCopy() const override;

    virtual string myName() const override;

    virtual void input(istream& is) override;
    virtual void output(ostream& os) const override;

    string getBank() const { return bank; }
};

class WebMoney : public Payment {
private:
    float commission;

public:
    WebMoney();
    WebMoney(const string& date, float amount, float commission);
    WebMoney(const WebMoney& other);
    virtual ~WebMoney();

    virtual Payment* getCopy() const override;

    virtual string myName() const override;
    
    virtual void input(istream& is) override;
    virtual void output(ostream& os) const override;

    float getCommission() const { return commission; }
};

class Sales {
private:
    vector<Payment*> payments;

public:
    Sales();
    Sales(const Sales& other);
    ~Sales();

    void addPayment(Payment* p);
    void displayAll() const;

    void selectByDate(const string& date) const;
    void selectByAmountRange(float min, float max) const;
    void showStatsByType() const;

    void loadFromFile(const string& filename);
    void saveToFile(const string& filename) const;
};

#endif