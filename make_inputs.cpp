#include"make_inputs.hpp"


int random_choice(const std::vector<int>& population, const std::vector<double>& weights, std::mt19937& gen) {
    if (population.empty() || population.size() != weights.size()) {
        throw std::runtime_error("Population and weights must have same size and be non-empty");
    }
    
    // Use the seeded generator passed from makeInputs
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    size_t index = dist(gen);
    
    return population[index];
}

vector<Message> makeInputs(int num_ecu,
                int num_bridges,
                int num_messages,
                int base_period,
                std::vector<int> period_choice,
                std::vector<double> period_choice_weights,
                int min_size,
                int max_size,
                int min_tl,
                int max_tl,
                int seed){

    std::srand(seed); // Seeds the standard rand()
    std::mt19937 gen(seed); // Seeds the mt19937 generator

    vector<Message> M(num_messages);
    for(int i = 0;i<num_messages;i++){
        int src = rand()%num_ecu;
        int sink = src; while(sink == src) sink = rand()%num_ecu;
        
        int range = max_size - min_size;
        int size = (range <= 0) ? min_size : (rand() % range) + min_size;
        
        // Pass the seeded generator here
        int period = random_choice(period_choice, period_choice_weights, gen) * base_period;
        
        int tl_range = max_tl - min_tl;
        int tl = (tl_range <= 0) ? min_tl : (rand() % tl_range) + min_tl;

        M[i] = {src,sink,size,period,tl};
    }
    
    return M;
}

void make_inputs(py::module_ &m) {
    py::bind_vector<std::vector<Message>>(m, "VectorMessage");
    py::bind_vector<std::vector<double>>(m, "Vectordouble");
    py::class_<Message>(m, "Message")
        .def(py::init<int,int,int,int,int>())
        .def_readwrite("src", &Message::src)
        .def_readwrite("sink", &Message::sink)
        .def_readwrite("size", &Message::size)
        .def_readwrite("period", &Message::period)
        .def_readwrite("tl", &Message::tl)
        .def("__repr__",&Message::to_string);

    m.def("makeInputs", &makeInputs,
        //   py::return_value_policy::take_ownership,
          py::arg("num_ecu"),
          py::arg("num_bridges"),
          py::arg("num_messages"),
          py::arg("base_period"),
          py::arg("period_choice"),
          py::arg("period_choice_weights"),
          py::arg("min_size"),
          py::arg("max_size"),
          py::arg("min_tl"),
          py::arg("max_tl"),
          py::arg("seed"));
}


