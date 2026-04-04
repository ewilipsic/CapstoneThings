#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/iostream.h>
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
#include<string>
#include<vector>
#include<map>
#include<set>
#include<iostream>
#include<algorithm>
#include<numeric>
#include <iomanip>
#include<unordered_map>
#include<cstdint>
#include <string>

#include"message.hpp"

typedef struct AlgoResults{
    int hyperperiod;
    std::vector<int> reps;
    std::vector<std::vector<int>> amount_sent;
    std::map<std::pair<int,int>,std::vector<std::vector<int>>> R; // pair(sorted msgidx,rep) -> routes
    std::map<std::pair<int,int>,std::vector<int>> departure_times; // pair(sorted msgidx,rep) -> depatrues of each disjoint
    std::vector<std::vector<int>> W;
    std::vector<float> point_array;
    std::vector<Message> sorted_messages;
    int Cost;

} AlgoResults;

namespace py = pybind11;
using namespace std;
    
PYBIND11_MAKE_OPAQUE(std::vector<int>)
PYBIND11_MAKE_OPAQUE(std::vector<float>)
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<int>>)
PYBIND11_MAKE_OPAQUE(std::map<std::pair<int,int>,std::vector<std::vector<int>>>)
PYBIND11_MAKE_OPAQUE(std::map<std::pair<int,int>,std::vector<int>>)

int nextUpgradeCost(int rank);
int CumulativeUpgradeCost(int rank);
void algo_bind(py::module_ &m);
void new_algo_bind(py::module_ &m);