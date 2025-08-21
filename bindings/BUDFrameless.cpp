#include "../src/UDFrameless.h"
#include<pybind11/pybind11.h>
namespace py=pybind11;

PYBIND11_MODULE(UDFrameless, mod) {
    mod.doc() = "cxxtestpy module";
    mod.def("setWindowEffect",&setWindowEffect,py::arg("hwnd"),py::arg("key"),py::arg("enable"));

    // mod.def("handle", &FooApi::instance);
}