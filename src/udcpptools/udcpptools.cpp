#include<pybind11/pybind11.h>
#include "udcpptools.h"

namespace py=pybind11;

Calculator::Calculator() : last_result_(0) {}

void Calculator::add(int value) {
    this->last_result_ += value;
}

void Calculator::subtract(int value) {
    this->last_result_ -= value;
}

int Calculator::getResult() const {
    return this->last_result_;
}

PYBIND11_MODULE(udcpptools,m){
    py::class_<Calculator>(m, "Calculator") 
        .def(py::init<>()) 
        .def("add", &Calculator::add, "Adds a value to the current result") 
        .def("subtract", &Calculator::subtract, "Subtracts a value") 
        .def("get_result", &Calculator::getResult, "Returns the current result"); 
}