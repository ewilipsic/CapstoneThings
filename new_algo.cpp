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
using namespace std;

typedef struct AlgoResults{
    int hyperperiod;
    std::vector<int> reps;
    std::vector<std::vector<int>> amount_sent;
    std::map<std::pair<int,int>,std::vector<std::vector<int>>> R; // pair(sorted msgidx,rep) -> routes
    std::map<std::pair<int,int>,std::vector<int>> departure_times; // pair(sorted msgidx,rep) -> depatrues of each disjoint
    std::vector<std::vector<int>> W;
    std::vector<float> point_array;
    int Cost;

} AlgoResults;

typedef struct Message {
    int src;
    int sink;
    int size;
    int period;
    int tl;

    std::string to_string() {
        return "[" + std::to_string(src) + "," +
                      std::to_string(sink) + "," +
                      std::to_string(size) + "," +
                      std::to_string(period) + "," +
                      std::to_string(tl) + "]";
    }
} Message;

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

// ----------------- in this file only ----------------------------


AlgoResults algo(int num_ecu,int num_bridges,vector<Message> M,int Bridge_limit,int link_build_cost,int yens_kmax,int assignment_type,int verbose,int debug_print){

    int HOP_COST = 1;
    int n = num_ecu + num_bridges;

    int hyper_period = 1;
    for(int i = 0;i<M.size();i++) hyper_period = lcm(hyper_period,M[i].period);
    if(verbose) cout<<"Hyper Period: "<<hyper_period<<endl;

    
    sort(M.begin(),M.end(),
    [&](const Message& m1,const Message& m2){
        if(m1.period != m2.period) return m1.period < m2.period;
        return m1.size > m2.size;
    });
    
    vector<int> repeats(M.size(),1);
    for(int i = 0;i<M.size();i++){
        repeats[i] = hyper_period/M[i].period;
    }
    // adj[t][u][v] link use between during the t'th time step 
    vector<vector<vector<int>>> adj(hyper_period,vector<vector<int>>(n,vector<int>(n, 0 )));
    for(int t = 0;t<hyper_period;t++){
        for(int node = 0;node < n;node++){
            adj[t][node][node] = 1;
        }
    }
    vector<vector<int>> W(n,vector<int>(n, link_build_cost + HOP_COST + CumulativeUpgradeCost(1) ));
    for(int u = 0;u<n;u++) W[u][u] = INT32_MAX;
    vector<int> node_rank(n);

    vector<vector<int>> amount_sent(M.size());
    for(int i = 0;i<M.size();i++){
        amount_sent[i] = vector<int>(repeats[i],0);
    }

    map<pair<int,int>,vector<vector<int>>> R; // pair(sorted msgidx,rep) -> routes
    map<pair<int,int>,vector<int>> departure_times; // pair(sorted msgidx,rep) -> depatrues of each disjoint

    for(int msg = 0;msg < M.size();msg++){
        cout<<"  "<<M[msg].to_string()<<"  "<<"Reps: "<<repeats[msg]<<endl;
        
        int src = M[msg].src;
        int sink = M[msg].sink;
        int size = M[msg].size;
        int tl = M[msg].tl;
        int period = M[msg].period;

        for(int rep = 0;rep < repeats[msg];rep++){

            int start_time = 0 + rep * M[msg].period;
            int end_time = 0 + rep * M[msg].period + M[msg].period;

            // --------------- handling a rep --------------------
            
            // (time,node) -> color -> vector cost,next node
            map< pair<int,int>, map< int, vector<pair<int,int>> > > nexts;
            int new_color = 0;

            // thing go from u -> v
            for(int v_time = end_time;v_time > start_time;v_time--){
                // propagate downwards
                for(int v = 0;v<n;v++){
                    if(v == src) continue;
                    if(v == sink){
                        for(int u = 0;u < n;u++){
                            if(u == sink) continue;
                            int tflag = 1;
                            for(int temp = v_time-1;temp < v_time - 1 + size;temp++){
                                if(temp >= hyper_period || adj[temp][u][v] != 0 || W[u][v] == INT32_MAX) tflag = 0;
                            }
                            if(!tflag) continue;

                            nexts[{v_time - 1,u}][new_color++].push_back({W[u][v],v});
                        }
                        continue;
                    }

                    for(int u = 0;u<n;u++){
                        if(u == v || u == sink) continue;
                        int tflag = 1;
                        for(int temp = v_time-1;temp < v_time - 1 + size;temp++){
                            if(temp >= hyper_period || adj[temp][u][v] != 0 || W[u][v] == INT32_MAX) tflag = 0;
                        }
                        if(!tflag) continue;

                        for(auto& cv : nexts[{v_time,v}]){
                            auto color = cv.first;
                            auto cost = cv.second[0].first + W[u][v];

                            nexts[{v_time-1,u}][color].push_back({cost,v});
                        }
                    }
                }

                // clarify nexts
                for(int u = 0;u<n;u++){
                    if(u == sink) continue;
        
                    vector<pair<int,int>> costcolor;
                    for(auto& cv: nexts[{v_time-1,u}]){
                        auto color = cv.first;
                        sort(cv.second.begin(),cv.second.end());
                        costcolor.push_back({cv.second[0].first,color});
                    }
                    sort(costcolor.begin(),costcolor.end());

                    int colors_done = 0;
                    set<int> used_nodes;
                    vector<int> colors_to_remove;

                    for(pair<int,int> p : costcolor){
                        auto [ min_cost_of_the_color ,color] = p;
                        if(colors_done >= tl){colors_to_remove.push_back(color);continue;}

                        int next_to_use = -1;
                        int next_to_use_cost = 0;
                        for(auto costNext : nexts[{v_time-1,u}][color]){
                            auto [cost_through_next ,Next] = costNext;
                            if(!used_nodes.count(Next)) {next_to_use = Next; next_to_use_cost = cost_through_next;}
                        }
                        if(next_to_use != -1) {
                            nexts[{v_time-1,u}][color] = vector<pair<int,int>> {{next_to_use_cost,next_to_use}} ;
                            colors_done++;
                            used_nodes.insert(next_to_use);
                        }
                        else colors_to_remove.push_back(color);                        
                    }

                    for(int color : colors_to_remove){
                        nexts[{v_time-1,u}].erase(color);
                    }
                }
            }

            // decide if possible
            set<int> possible_colors;
            for(int u_time = end_time - 1;u_time >= start_time;u_time--){
                for(auto cv : nexts[{u_time,src}]){
                    possible_colors.insert(cv.first);
                }
            }

            // rep is unschedulable
            if(possible_colors.size() < tl) continue;

            // schedule rep
            // color -> cheapest cost
            map<int,int> ccost;
            // color -> Cheapest starttime
            map<int,int> cstarttime;
            for(int u_time = end_time - 1;u_time >= start_time;u_time--){
                for(auto cv : nexts[{u_time,src}]){
                    if((ccost.count(cv.first) && (ccost[cv.first] > cv.second[0].first)) || (!ccost.count(cv.first))){
                        ccost[cv.first] = cv.second[0].first;
                        cstarttime[cv.first] = u_time;
                    }
                }
            }

            vector<pair<int, int>> sorted_colors;

            // 1. Populate the vector with the (color, start_time) pairs
            for (const auto& [color, start_time] : cstarttime) {
                sorted_colors.push_back({color, start_time});
            }

            // 2. Sort the vector using a lambda function to compare costs from the ccost map
            sort(sorted_colors.begin(), sorted_colors.end(), 
                [&ccost](const pair<int, int>& a, const pair<int, int>& b) {
                    if (ccost[a.first] == ccost[b.first]) {
                        // Optional tie-breaker: order by earlier start_time if costs are equal
                        return a.second < b.second; 
                    }
                    // Primary sort: order by cheapest cost ascending
                    return ccost[a.first] < ccost[b.first];
                }
            );

            int tls_done = 0;
            vector<vector<int>> Rm;
            vector<int> departure_timesm;

            for(auto [c, starttime] : sorted_colors){
                departure_timesm.push_back(starttime);
                vector<int> route;

                route.push_back(src);
                int current_node = src;
                int current_time = starttime;
                while(current_node != sink){
                    current_node = nexts[{current_time,current_node}][c][0].second;
                    route.push_back(current_node);
                    current_time++;
                }

                Rm.push_back(route);
                tls_done++;
                if(tls_done >= tl) break;
            }

            // update weights
            for(int idx = 0;idx<Rm.size();idx++){
                vector<int>& route = Rm[idx];
                for(int i = 0;i<route.size()-1;i++){
                    for(int s = 0; s < size;s++) adj[departure_timesm[idx] + i + s][route[i]][route[i+1]] = 1;
                    if(W[route[i]][route[i+1]] != HOP_COST) {node_rank[route[i]]++;node_rank[route[i+1]]++;}
                    W[route[i]][route[i+1]] = HOP_COST;
                    W[route[i+1]][route[i]] = HOP_COST;
                }
            }

            // W
            for(int u = 0; u<n; u++){
                int src_cost = HOP_COST + link_build_cost + nextUpgradeCost(node_rank[u]);
                for(int v = u+1;v <n;v++){
                    if(u == v || W[u][v] == HOP_COST) continue;
                    int dst_cost = nextUpgradeCost(node_rank[v]);
                    int new_cost = (node_rank[u] > Bridge_limit || node_rank[v] > Bridge_limit) ? INT32_MAX : src_cost + dst_cost;
                    W[u][v] = new_cost;
                    W[v][u] = new_cost;
                }
            }

            // update amount sent,R,depature_times
            amount_sent[msg][rep] = tl;
            R[{msg,rep}] = Rm;
            departure_times[{msg,rep}] = departure_timesm;


        }
    }

    // Quash HOP COST to 0 in W to maintain similiar output as other algos
    for(int u = 0;u<n;u++){
        for(int v = 0;v<n;v++){
            if(W[u][v] == HOP_COST) W[u][v] = 0;
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

    AlgoResults ret = {hyper_period,repeats,amount_sent,R,departure_times,W,{},topologyCost};

    return ret;

}

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

int main() {
    // 1. Define inputs for makeInputs
    int num_ecu = 4;
    int num_bridges = 2;
    int num_messages = 3;
    int base_period = 1;
    std::vector<int> period_choice = {2, 3};
    std::vector<double> period_choice_weights = {0.5, 0.3};
    int min_size = 1;
    int max_size = 1;
    int min_tl = 2;
    int max_tl = 2;
    int seed = 42;

    // 2. Generate Messages
    vector<Message> messages = makeInputs(
        num_ecu, num_bridges, num_messages, base_period,
        period_choice, period_choice_weights,
        min_size, max_size, min_tl, max_tl, seed
    );

    // vector<Message> messages = {
    //     {1,0,1,1,2},
    //     {3,0,1,3,2},
    // };

    cout << "Generated Messages:" << endl;
    for (Message& m : messages) {
        cout << m.to_string() << endl;
    }
    cout << "-----------------------" << endl;

    // 3. Define inputs for algo
    int Bridge_limit = 3;
    int link_build_cost = 2;
    int yens_kmax = 5;
    int assignment_type = 1;
    int verbose = 1;
    int debug_print = 0;

    // 4. Run algo
    AlgoResults results = algo(
        num_ecu, num_bridges, messages, Bridge_limit, 
        link_build_cost, yens_kmax, assignment_type, verbose, debug_print
    );

    cout << "-----------------------" << endl;
    cout << "Algorithm Execution Complete." << endl;
    cout << "Hyperperiod calculated: " << results.hyperperiod << endl;
    cout << "Total Topology Cost: " << results.Cost << endl;
    cout << "Routes stored for " << results.R.size() << " message/rep combinations." << endl;

    // Print Routes (R)
    cout << "\n--- Routes (R) ---" << endl;
    for (const auto& [key, routes] : results.R) {
        cout << "Msg " << key.first << ", Rep " << key.second << ":" << endl;
        for (int i = 0; i < routes.size(); ++i) {
            cout << "Start Time: " << results.departure_times[key][i] << "  Path " << i + 1 << ": ";
            for (int j = 0; j < routes[i].size(); ++j) {
                cout << routes[i][j];
                if (j < routes[i].size() - 1) cout << " -> ";
            }
            cout << endl;
        }
    }

    // Print Weights Matrix (W)
    cout << "\n--- Topology Weights Matrix (W) ---" << endl;
    int n = results.W.size();
    
    // Print column headers
    cout << "    ";
    for (int i = 0; i < n; ++i) {
        cout << setw(5) << i;
    }
    cout << "\n    " << string(n * 5, '-') << endl;

    // Print rows
    for (int i = 0; i < n; ++i) {
        cout << setw(2) << i << " |";
        for (int j = 0; j < n; ++j) {
            if (results.W[i][j] == INT32_MAX) {
                cout << setw(5) << "INF";
            } else {
                cout << setw(5) << results.W[i][j];
            }
        }
        cout << endl;
    }


    return 0;
}