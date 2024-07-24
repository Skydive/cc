#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <algorithm>
#include <tuple>

int main(int argc, char** argv) {
    // n ingredients, k grams magic powder
    // a1, a2, ... an: ai grams of ith ingredient needed to bake 1 cookie
    // b1, b2, ... bn: bi grams of ith ingredient that Apollinaria has
    
    // Case 1: 
    // INPUT:
    // 3 1
    // 2 1 4
    // 11 3 16

    // Pivots:
    // (5,1) (3, 0), (4, 0)
    // Max 3 cookies!

    // Take smallest result:
    // 3 + 1(powder) = 4 --> 4 cookies

    // Case 2:
    // Input:
    // 4 3
    //  4  3  5  6
    // 11 12 14 20
    // (2, 3), (4, 0), (2, 4), (3, 2)
    // (2, 1), (4, 0), (2, 1), (3, 4)
    // If I give my powder -> idx 0, 3
    // --> 3 4 3 3 --> 3 cookies

    // Sort by smallest number O(nlogn)
    // Sort by remainder O(klogk)
    // Fill bins O(k)
    
    // Let's have a go!
    using namespace std;
    int n, k;
    vector<int> a(n), b(n);

    cin >> n; cin >> k;
    for(int i=0; i<n; i++) cin >> a[i];
    for(int i=0; i<n; i++) cin >> b[i];

    // Maybe some sort of priority queue...

    typedef tuple<int, int, int> ti; 
    vector<ti> c(n);
    for(int i=0; i<n; i++) {
        auto [q, r] = div(b[i], a[i]);
        c[i] = {q, a[i]-r, i};
        // cout << q << "," << a[i]-r << endl;
    }
    sort(c.begin(), c.end());
    // cout << "Sorted C:" << endl;
    for(int i=0; i<n; i++) {
        auto [x, kk, j] = c[i];
        // cout << x << "-" << kk << "-" << j << endl;        
    }
    
    priority_queue<ti, vector<ti>, greater<ti>> p(c.begin(), c.end());
    while(k > 0) {
        // OR: Binary search for element then rebuild the list.
        // O(n) + O(logn)
        auto [x, kk, i] = p.top();
        // cout << "Top: " << x << "-" << kk << "-" << i << endl;
        if(k >= kk) {
            p.pop();
            p.push({x+1, a[i]-1, i});
            k -= kk;
        } else {
            break; // makes no difference
        }
    }
    auto [x, kk, i] = p.top();
    // cout << "Last: " << x << "-" << kk << "-" << i << endl;
    // cout << x << endl;
    return 0;
}
