// https://codeforces.com/problemset/problem/20/C

#include <iostream>
#include <vector>
#include <queue> // priority_queue
#include <algorithm>

using namespace std;

int main(int argc, char** argv) {
    int n, m;
    cin >> n >> m;
    using edge_t = pair<int, int>;
    vector<vector<edge_t>> neighbors(n+1, vector<edge_t>{});
    for(int i=0; i<m; i++) {
        int a, b, w;
        cin >> a >> b >> w;
        neighbors[a].push_back({b, w});
        neighbors[b].push_back({a, w});
    }
    
    vector<bool> seen(n+1, false);
    vector<int> min_dist(n+1, -1);
    vector<int> prev(n+1, -1);
    using weight_edge_t = pair<int, int>;
    priority_queue<weight_edge_t, vector<weight_edge_t>, greater<weight_edge_t>> pq;
    pq.push({0, 1});
    min_dist[1] = 0;
    prev[1] = -1;

    while(!pq.empty()) {
        auto [d, cur_n] = pq.top();
        pq.pop();
        if(seen[cur_n])continue;
        seen[cur_n] = true;
        if(cur_n == n)break;
        for(auto [nn, e] : neighbors[cur_n]) {
            if(!seen[nn] && d+e < min_dist[nn]) {
                prev[nn] = cur_n;
                pq.push({d+e, nn});
                min_dist[nn] = d+e    
            }

        }
    }
    vector<int> prev_chain{n};
    int cur_n = n;
    while(cur_n != 1) {
        cur_n = prev[n];
        prev_chain.push_back(cur_n);
    }
    reverse(prev_chain.begin(), prev_chain.end());
    for(int cur_n : prev_chain)
        cout << cur_n << " ";
    cout << endl;
    return 0;
}