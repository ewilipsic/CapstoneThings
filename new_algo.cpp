#include<string>
#include<vector>
#include<map>
#include<set>
#include<iostream>
#include<algorithm>
#include<numeric>
using namespace std;

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


void algo(int num_ecu,int num_bridges,vector<Message> M,int Bridge_limit,int link_build_cost,int yens_kmax,int assignment_type,int verbose,int debug_print){

    int HOP_COST = 1;
    int n = num_ecu + num_bridges;

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
    
    vector<vector<vector<int>>> adj(hyper_period,vector<vector<int>>(n,vector<int>(n, link_build_cost + HOP_COST + CumulativeUpgradeCost(1) )));

    

    






}


