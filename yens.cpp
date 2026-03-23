#include"yens.hpp"

using namespace std;


vector<int> return_shortest_path(int n,vector<vector<int>>& adj,int source,int sink,int& cost){

    vector<int> min_dist(n,INT32_MAX);
    vector<int> prev(n,-1);
    min_dist[source] = 0;
    priority_queue<pair<int,int>, vector<pair<int,int>>, std::greater<pair<int,int>>> pq;

    pq.push({0,source});

    while(!pq.empty()){
        auto [dist,node] = pq.top();pq.pop();
        if(dist != min_dist[node]) continue;
        if(node == sink) break;
       
        
        for(int i = 0;i<n;i++){
            if(adj[node][i] == INT32_MAX) continue;
            if(min_dist[i] > dist + adj[i][node]){
                min_dist[i]  = dist + adj[i][node];
                pq.push({min_dist[i],i});
                prev[i] = node;
            }
        }
    }
    
    vector<int> route;
    int cur = sink;
    while(true){
        route.push_back(cur);
        if(prev[cur] == -1) break;
        cur = prev[cur];
    }
    reverse(route.begin(), route.end());

    cost = min_dist[sink];
    if(cur == source) return route;
    return {};
}

vector<int> return_shortest_path_vis(int n,vector<vector<int>>& adj,int source,int sink,int& cost,vector<int>& vis_arr,int visi){

    vector<int> min_dist(n,INT32_MAX);
    
    for(int i = 0;i<= visi;i++) min_dist[vis_arr[i]] = 0;
    vector<int> prev(n,-1);
    min_dist[source] = 0;
    priority_queue<pair<int,int>, vector<pair<int,int>>, std::greater<pair<int,int>>> pq;

    pq.push({0,source});

    while(!pq.empty()){
        auto [dist,node] = pq.top();pq.pop();
        if(dist != min_dist[node]) continue;

        if(node == sink) break;

        for(int i = 0;i<n;i++){
            if(adj[node][i] == INT32_MAX) continue;
            if(min_dist[i] > dist + adj[i][node]){
                min_dist[i]  = dist + adj[i][node];
                pq.push({min_dist[i],i});
                prev[i] = node;
            }
        }
    }

    vector<int> route;
    int cur = sink;
    while(true){
        route.push_back(cur);
        if(prev[cur] == -1) break;
        cur = prev[cur];
    }
    reverse(route.begin(), route.end());

    cost = min_dist[sink];
    if(cur == source) return route;
    return {};
}

void yens(int n,
    vector<vector<int>>& adj,int source,int sink,int K,
    vector<int>& A,
    int& prev_cost,
    int& prev_spuridx,
    priority_queue<pair<int,int>,vector<pair<int,int>>,std::greater<pair<int,int>>>& B_queue,
    map<int,int>& B_spuridx,
    int& key,
    map<int,vector<int>>& B_paths){

    if(K == 0){
        int cost;
        A = return_shortest_path(n,adj,source,sink,cost);
        prev_cost = cost;
        prev_spuridx = -1;
        return;
    }

    
    vector<int> spurToable(n,1);
    for(int node : A) spurToable[node] = 0;
    spurToable[sink] = 1;

    // select spur vertex
    for(int i = (int)A.size() - 2;i >= max(0,prev_spuridx + 1);i--){
        int old = adj[A[i]][A[i+1]];
        prev_cost -= old;
        adj[A[i]][A[i+1]] = INT32_MAX;
        adj[A[i+1]][A[i]] = INT32_MAX;

        for(int spurto = 0;spurto < n;spurto++){
            if(adj[A[i]][spurto] == INT32_MAX || spurToable[spurto] == 0) continue;
            int spur_cost = 0;
            vector<int> spur_path = return_shortest_path_vis(n,adj,spurto,sink,spur_cost,A,i);

            B_queue.push({prev_cost + adj[A[i]][spurto] + spur_cost,key});
            B_spuridx[key] = i;
            vector<int> new_path;new_path.reserve(i + 1 + spur_path.size());
            for(int j = 0;j<=i;j++) new_path.push_back(A[j]);
            for(int j = 0; j <spur_path.size();j++) new_path.push_back(spur_path[j]);
            B_paths[key] = new_path;
            key++;
        }

        spurToable[A[i+1]] = 1;
        adj[A[i]][A[i+1]] = old;
        adj[A[i+1]][A[i]] = old;
    }
 
    // give new A
    // cout<<"B size before: "<<B_paths.size()<<endl;
    if(B_queue.empty()){
        prev_cost = INT32_MAX;
        A = {};
        prev_spuridx = -1;
        // cout<<"B unable size: "<<B_paths.size()<<endl;
        return;
    }
    auto [cost,new_path_key] = B_queue.top();B_queue.pop();
    A = B_paths[new_path_key];
    prev_cost = cost;
    prev_spuridx = B_spuridx[new_path_key];
    
    B_spuridx.erase(new_path_key);
    B_paths.erase(new_path_key);
    // cout<<"B size: "<<B_paths.size()<<endl;
}

void print_vec(vector<int>& arr){
    for(int i : arr) cout<<i<<" ";
    cout<<endl;
}

// int main(){
//     int n = 6;
//     vector<vector<int>> adj = {
//     {INT32_MAX, 1, 1, INT32_MAX, INT32_MAX, 1},
//     {1, INT32_MAX, INT32_MAX, INT32_MAX, INT32_MAX, 1},
//     {1, INT32_MAX, INT32_MAX, 1, 1, 1},
//     {INT32_MAX, INT32_MAX, 1, INT32_MAX, 1, 1},
//     {INT32_MAX, INT32_MAX, 1, 1, INT32_MAX, 1},
//     {1, 1, 1, 1, 1, INT32_MAX}
// };

//     int source = 0;
//     int sink = 5;
//     int K = 0;
//     vector<int> A = {};
//     int prev_cost = 0;
//     int prev_spuridx = -1;
//     priority_queue<pair<int,int>,vector<pair<int,int>>,std::greater<pair<int,int>>> B_queue;
//     map<int,int> B_spuridx;
//     int key = 0;
//     map<int,vector<int>> B_paths;

//     while(prev_cost != INT32_MAX){
//         yens(
//         n,
//         adj,source,sink,K,
//         A,
//         prev_cost,
//         prev_spuridx,
//         B_queue,
//         B_spuridx,
//         key,
//         B_paths);

//         K++;

        
//         cout<<"Cost "<<prev_cost<<endl;
//         cout<<"Path: ";
//         print_vec(A);
    
//     }
// }
