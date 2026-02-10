#ifndef FIGURE_EXCEPTIONS_H
#define FIGURE_EXCEPTIONS_H

#include <exception>
#include <string>

class FigureException : public std::exception {
private:
    std::string message;
public:
    FigureException(const std::string& msg) : message("Ошибка фигуры: " + msg) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class ValidationError : public FigureException {
public:
    ValidationError(const std::string& msg) : FigureException("Ошибка валидации - " + msg) {}
};
#endif