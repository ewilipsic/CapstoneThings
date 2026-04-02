#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;
using namespace std;

void make_inputs(py::module_ &m);
void algo_bind(py::module_ &m);
void new_algo_bind(py::module_ &m);

PYBIND11_MODULE(tsn, m, py::mod_gil_not_used()) {
    make_inputs(m);
    algo_bind(m);
    new_algo_bind(m);
}