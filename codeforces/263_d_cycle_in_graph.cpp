
#include <iostream>
#include <vector>
#include <algorithm>

#include <unordered_map>
// #include <unordered_set>

using namespace std;
typedef unordered_map<int, vector<int>> neighbors_t;

bool dfs(int i, int prev, neighbors_t& neighbors, vector<int>& state, vector<int>& path, int k) {
    state[i] = 1;
    path.push_back(i);
    for(int n : neighbors[i]) {
        if(n == prev) continue; // since undirected
        if(state[n] == 1) { // We visited this on THIS iteration! :)
            if(path.size() >= k+1) { // Since current node repeats, cycle of length at least k+1
                return true;
            }
        } else if(state[n] = 0) {
            bool res = dfs(n, i, neighbors, state, path, k);
            if(res) {
                return true; // Exit if we found a cycle 
            }
        }
    }
    path.pop_back();
    state[i] = 2; // Mark as visited when we exit our iteration
    return false;
}

int main(int argc, char** argv) {
    int n, m, k;
    cin >> n; cin >> m; cin >> k;
    neighbors_t neighbours;
    for(int i=0; i<m; i++) {
        int s, t;
        cin >> s; cin >> t;
        neighbours[s].push_back(t); // Indexing automatically creates an empty vector
        neighbours[t].push_back(s);
    }

    vector<int> state(n+1, 0); // 0 - unvisited, 1 - visiting, 2 - visited
    vector<int> path;
    for(int i=1; i<=n; i++) {
        if(state[i] == 0) {
            path.clear();
            if(dfs(i, -1, neighbours, state, path, k)) {
                cout << path.size() << endl;
                for(int x : path)cout << x << " ";
                cout << endl;
                break;
            }
        }
    }
    return 0;
}
