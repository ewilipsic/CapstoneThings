#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <set>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <map>
#include <numeric>
#include <algorithm>
#include <queue>
#include <cstdint>
#include <string>
#include <random>


#include"message.hpp"

std::vector<Message> makeInputs(int num_ecu,
                int num_bridges,
                int num_messages,
                int base_period,
                std::vector<int> period_choice,
                std::vector<double> period_choice_weights,
                int min_size,
                int max_size,
                int min_tl,
                int max_tl,
                int seed);

namespace py = pybind11;
using namespace std;
    
PYBIND11_MAKE_OPAQUE(std::vector<Message>)
PYBIND11_MAKE_OPAQUE(std::vector<double>)
void make_inputs(py::module_ &m);
