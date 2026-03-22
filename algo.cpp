#include"algo.hpp"
#include"yens.hpp"
using namespace std;

int assignMessage(Message& msg,vector<int>& route,int start_time,int end_time,map<pair<int,int>,set<int>>& S,int& departure_time,int hyperperiod,int assignment_type = 0){
    /*
        up in the schedule means time is increasing
        assignment type = 0 First Fit
        assignment type = 1 Best Fit Minimize fragmentation by 
            maximizing the difference between min (free time above) and min (free time below) the route
    */

    if(assignment_type == 1){
    int best_time = -1;
    int best_swing = -1;

    for(int t = start_time;t <= end_time;t++){
        int tflag = 1;
        int free_up = hyperperiod;
        int free_below = hyperperiod;

        for(int i = 0;i<(int)route.size()-1;i++){

            int u = (route[i] < route[i+1]) ? route[i] : route[i+1];
            int v = (route[i] < route[i+1]) ? route[i+1] : route[i];
            for(int lt = t + i;lt < t + i + msg.size ; lt++){
                if(lt > end_time) tflag = 0;
                if(S[{u,v}].count(lt)) tflag = 0;
            }

            if(tflag == 0) break;
            auto It_occupancy_up = S[{u,v}].upper_bound(t + i + msg.size - 1);
            int occupancy_up = (It_occupancy_up != S[{u,v}].end()) ? *It_occupancy_up : hyperperiod;
            free_up = min(free_up, occupancy_up - (t + i + msg.size - 1) - 1);

            auto It_occupancy_below = S[{u,v}].lower_bound(t + i);

                // case of S[{u,v}] is handled as then begin() == end()
            int occupancy_below;
            if (It_occupancy_below == S[{u,v}].begin()) occupancy_below = -1;
            else occupancy_below = *(--It_occupancy_below);
            free_below = min(free_below, (t + i) - occupancy_below - 1);
        }
        
        if(tflag && abs(free_up - free_below) > best_swing){
            best_swing = abs(free_up - free_below);
            best_time = t;
        }
    }

    if(best_time == -1) return 0;
    departure_time = best_time;
    for(int i = 0;i<(int)route.size()-1;i++){
        int u = (route[i] < route[i+1]) ? route[i] : route[i+1];
        int v = (route[i] < route[i+1]) ? route[i+1] : route[i];
        for(int lt = best_time + i;lt < best_time + i + msg.size ; lt++){
            S[{u,v}].insert(lt);
        }
    }

    return 1;
    }
    

    if(assignment_type == 0){
    for(int t = start_time;t <= end_time;t++){
        int tflag = 1;
        for(int i = 0;i<(int)route.size()-1;i++){
            int u = (route[i] < route[i+1]) ? route[i] : route[i+1];
            int v = (route[i] < route[i+1]) ? route[i+1] : route[i];
            for(int lt = t + i;lt < t + i + msg.size ; lt++){
                if(lt > end_time) tflag = 0;
                if(S[{u,v}].count(lt)) tflag = 0;
            }
            if(tflag == 0) break;
        }
        if(tflag){
            departure_time = t;
            for(int i = 0;i<(int)route.size()-1;i++){
                int u = (route[i] < route[i+1]) ? route[i] : route[i+1];
                int v = (route[i] < route[i+1]) ? route[i+1] : route[i];

                for(int lt = t + i;lt < t + i + msg.size ; lt++){
                    S[{u,v}].insert(lt);
                }
            }

            return 1;
        }
    }
    }

    return 0;
}

int nextUpgradeCost(int rank){
    return 2;
}

int CumulativeUpgradeCost(int rank){
    int cost = 0;
    for(int i = 0; i < rank; i++){
        cost += nextUpgradeCost(i);
    }
    return cost;
}

void updateWeights(int num_ecu,int num_bridges,vector<int>& route,vector<vector<int>>& W,vector<vector<int>>& Wm,vector<int>& node_rank,int Bridge_Limit = 3,int link_build_cost = 2){
    for(int i = 0;i<(int)route.size()-1;i++){
        
        int u = route[i];
        int v = route[i+1];

        if(W[u][v] == 0) continue;

        W[u][v] = 0;
        W[v][u] = 0;
        node_rank[u]++;
        node_rank[v]++;
    }


    for(int i = 0;i<route.size();i++){

        int node = route[i];
        int next_cost;
        // this price is arbitary
        int src_cost = (node_rank[node] < Bridge_Limit)? nextUpgradeCost(node_rank[node]) + link_build_cost : INT32_MAX;
        for(int j = 0;j<num_ecu+num_bridges;j++){
            if(j == node || W[node][j] == 0) continue;
            int dest_cost = (node_rank[j] < Bridge_Limit)? nextUpgradeCost(node_rank[j]) : INT32_MAX;
            if (src_cost == INT32_MAX || dest_cost == INT32_MAX) next_cost = INT32_MAX;
            else next_cost = dest_cost + src_cost;

            W[node][j] = next_cost;
            W[j][node] = next_cost;

            // since Wm has infs for disjointness
            Wm[node][j] = max(Wm[node][j],next_cost);
            Wm[j][node] = max(Wm[j][node],next_cost);
        }
    }
};

AlgoResults algo(int num_ecu,int num_bridges,vector<Message> M,int Bridge_limit,int link_build_cost,int assignment_type,int verbose,int debug_print){

    if(debug_print) cout<<"ALGO IN"<<endl;
    vector<vector<int>> W(num_ecu+num_bridges,vector<int>(num_bridges + num_ecu,1));
    vector<int> node_rank(num_ecu+num_bridges,0);
    for(int i = 0;i < num_bridges+num_ecu;i++) W[i][i] = INT32_MAX;
    
    int hyper_period = 1;
    for(int i = 0;i<M.size();i++) hyper_period = lcm(hyper_period,M[i].period);
    if(verbose) cout<<"Hyper Period: "<<hyper_period<<endl;
    vector<int> repeats(M.size(),1);
    for(int i = 0;i<M.size();i++){
        repeats[i] = hyper_period/M[i].period;
    }
    
    sort(M.begin(),M.end(),
    [&](const Message& m1,const Message& m2){
        if(m1.period != m2.period) return m1.period < m2.period;
        return m1.size > m2.size;
    });

    vector<vector<int>> amount_sent(M.size());
    for(int i = 0;i<M.size();i++){
        amount_sent[i] = vector<int>(repeats[i],0);
    }
    
    map<pair<int,int>,vector<vector<int>>> R; // pair(sorted msgidx,rep) -> routes
    map<pair<int,int>,vector<int>> departure_times; // pair(sorted msgidx,rep) -> depatrues of each disjoint
    map<pair<int,int>,set<int>> S; // link(u,v) u < v -> set of times where it is occupied
    
    for(int msg = 0;msg < M.size();msg++){
        // cout<<"Message: "<<msg<<" Total Reps: "<<repeats[msg]<<" Tl: "<<M[msg].tl<<endl;
        for(int rep = 0;rep < repeats[msg];rep++){
            // cout<<rep<<endl;
            
        vector<vector<int>> Rm;
        vector<int> deps;
        vector<vector<int>> Wm = W;
        
        int start_time = 0 + rep * M[msg].period;
        int end_time = 0 + rep * M[msg].period + M[msg].period - 1;
        
        for(int disjoint = 0;disjoint < M[msg].tl ;disjoint++){
            // cout<<"d"<<disjoint<<endl;
            if(Rm.size()){
                for(int node = 0;node < Rm[Rm.size() - 1].size() - 1;node++){
                    Wm[Rm[Rm.size() - 1][node]][Rm[Rm.size() - 1][node + 1]] = INT32_MAX;
                    Wm[Rm[Rm.size() - 1][node + 1]][Rm[Rm.size() - 1][node]] = INT32_MAX;
                }
            }
            
            int flag = 0,k = 0;
            vector<int> A;
            int prev_cost = INT32_MAX;
            int prev_spuridx = -1;
            
            int B_key = 0;
            
            std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> B_queue;
            std::map<int, int> B_spuridx;
            std::map<int, std::vector<int>> B_paths;
            int departure_time = -1;
            
            while(flag == 0){
                // cout<<"yen k"<<k<<endl;
                if(debug_print) cout<<"YENS IN"<<endl;
                yens(   num_bridges + num_ecu,Wm,
                    M[msg].src,M[msg].sink,k,
                    A,prev_cost,prev_spuridx,
                    B_queue,B_spuridx,B_key,B_paths
                );
                k = k + 1;
                if(debug_print) cout<<"YENS OUT"<<endl;
                if(prev_cost == INT32_MAX){
                    prev_cost = INT32_MAX;
                    // cout<<"yen break"<<endl;
                    break;
                }
                
                // print_vec(A);
                flag = assignMessage(M[msg],A,start_time,end_time,S,departure_time,hyper_period,assignment_type);
            }

            // No paths left therefore no other disjoint paths exist
            if(flag == 0) break;

            amount_sent[msg][rep]++;
            Rm.push_back(A);
            deps.push_back(departure_time);
            // cout<<"House keeping issue"<<endl;
        
            updateWeights(num_ecu,num_bridges,A,W,Wm,node_rank,Bridge_limit,link_build_cost);
         
        }
        
        // cout<<"Clean up issue"<<endl;
        
        R[{msg,rep}] = Rm;
        departure_times[{msg,rep}] = deps;
        
    }
    }


    
    int topologyCost = 0;
    for(int i = 0;i<num_ecu+num_bridges;i++){
        for(int j = 0;j<num_ecu+num_bridges;j++){
            if(W[i][j] == 0) topologyCost += link_build_cost;
        }
    }
    topologyCost = topologyCost/2;
    for(int i = 0;i<num_ecu+num_bridges;i++) topologyCost += CumulativeUpgradeCost(node_rank[i]);
    
 
    
    
    if(verbose){
        for(Message m : M){
            cout<<"Src: "<<m.src<<" ";
            cout<<"Sink: "<<m.sink<<" ";
            cout<<"Size: "<<m.size<<" ";
            cout<<"Period: "<<m.period<<" ";
            cout<<"TL: "<<m.tl<<"\n";
        }
        cout<<"\n";
        
        cout<<"TopologyCost: "<<topologyCost<<"\n";
        
        for(int i = 0;i<M.size();i++){
            for(int rep = 0; rep< repeats[i];rep++){
                cout<<"msg: "<<i<<" rep: "<<rep<<"\n";
                cout<<"Paths\n";
                for(int j = 0;j<amount_sent[i][rep];j++){
                    cout<<"Start Time: "<<departure_times[{i,rep}][j]<<endl;
                    cout<<"Path: ";
                    for(int x : R[{i,rep}][j]) cout<<x<<" ";
                    cout<<"\n";
                }
                cout<<"\n";
            }
        }
    }
    
    AlgoResults ret = {hyper_period,repeats,amount_sent,R,departure_times,W,topologyCost};
    if(debug_print) cout<<"ALGO OUT"<<endl;
    return ret;
    
}

void algo_bind(py::module_ &m) {
    py::bind_vector<std::vector<int>>(m, "VectorInt");
    py::bind_vector<std::vector<std::vector<int>>>(m, "VectorVectorInt");
    py::bind_map<std::map<std::pair<int,int>,std::vector<std::vector<int>>>>(m,"Routes");
    py::bind_map<std::map<std::pair<int,int>,std::vector<int>>>(m,"DepartureTimes");

    py::class_<AlgoResults>(m, "AlgoResults")
        .def_readwrite("hyperperiod", &AlgoResults::hyperperiod)
        .def_readwrite("reps", &AlgoResults::reps)
        .def_readwrite("amount_sent", &AlgoResults::amount_sent)
        .def_readwrite("R", &AlgoResults::R)
        .def_readwrite("departure_times", &AlgoResults::departure_times)
        .def_readwrite("W", &AlgoResults::W)
        .def_readwrite("Cost", &AlgoResults::Cost);

    m.def("algo", [](int num_ecu, int num_bridges, vector<Message> M, 
                 int Bridge_limit, int link_build_cost, 
                 int assignment_type, int verbose, int debug_print) {
    
    // This guard redirects std::cout to sys.stdout while this lambda is executing
    py::scoped_ostream_redirect stream(
        std::cout,                                
        py::module_::import("sys").attr("stdout") 
    );

    return algo(num_ecu, num_bridges, M, Bridge_limit, 
                link_build_cost, assignment_type, verbose, debug_print);
    },
    // py::return_value_policy::take_ownership,
    py::arg("num_ecu"),
    py::arg("num_bridges"),
    py::arg("MessageVector"),
    py::arg("Bridge_Limit") = 3,
    py::arg("link_build_cost") = 2,
    py::arg("assignment_type") = 0,
    py::arg("verbose") = 0,
    py::arg("debug_print") = 0);
}

// int main(){

//     int num_ecu = 7,num_bridges = 10,num_msseages = 10;
//     vector<Message> M = makeInputs(num_ecu,num_bridges,num_msseages,2,4,1,2,1,2,42);
//     algo(num_ecu,num_bridges,M);

// }
