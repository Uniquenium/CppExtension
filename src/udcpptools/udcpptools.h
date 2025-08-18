#pragma once
#ifndef UNIDESKTOOLS_H
#define UNIDESKTOOLS_H
class Calculator {
private:
    int last_result_;

public:
    Calculator();
    void add(int value);
    void subtract(int value);
    int getResult() const;
};
#endif