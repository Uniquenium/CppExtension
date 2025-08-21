#include "../src/UDFrameless.h"
#include<pybind11/pybind11.h>
namespace py=pybind11;

PYBIND11_MODULE(UDFrameless,m){
    m.doc()="11111";
    // m.def("setWindowEffect",&setWindowEffect,py::arg("hwnd"),py::arg("key"),py::arg("enable"));
}