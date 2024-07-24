
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, char** argv) {
    int n; cin >> n;
    vector<int> xs(n);
    for(int i=0; i<n; i++)cin >> xs[i];
    int q; cin >> q;

    // Number of shops, order doesn't matter
    sort(xs.begin(), xs.end());
    for(int i=0; i<q; i++) {
        int m;cin >> m;
        // cout << "m: " << m << endl;
        // Do binary search, output index
        if(m < xs[0]) {
            cout << 0 << endl; continue;
        } else if(m > xs[n-1]) {
            cout << n << endl; continue;
        }

        // C++ binary search in vector: lower_bound, upper_bound
        // upper_bound(f, l, v) - iterator pointing to first element in range [f, l) GREATER THAN than val, or .end()
        // lower_bound(f, l, v) - iterator pointing to first element in range [f, l) GREATER THAN OR EQ TO [NOT LESS THAN] val, or .end()

        // Find first element GREATER THAN m = 7
        // 6 8 9 --> 8 -> IDX
        // 6 7 8 --> 

        int lo=0,hi=n;
        while(lo < hi) { 
            int mid = lo + (hi-lo)/2;
            // cout << lo << "," << hi << "->" << mid << endl;
            if(xs[mid] <= m) {
                lo = mid+1;
            } else {
                hi = mid;
            }
        }
        // if(lo < n && xs[lo] <= m) // m > xs[n-1]
            // lo++;


        // lo = distance(xs.begin(), upper_bound(xs.begin(), xs.end(), m));
        cout << lo << endl;

    }
    return 0;
}
