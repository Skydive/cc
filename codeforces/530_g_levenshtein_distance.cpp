
#include <iostream>
#include <vector>
#include <string>

#include <algorithm>

using namespace std;

int main(int argc, char** argv) {
    string a, b;
    cin >> a; cin >> b;

    // f(i, j)
    // if a[i] == b[j] -> = f(i-1, j-1) same as edit distance to make a[:i-1] -> b[:i-1]    
    // else: MIN OF
    // insertion: = (index number of inserted letter b[j]) + f(i, j-1)
    // deletion: = (index number of deleted letter a[i]) + f(i-1, j)
    // substitution: = (difference of index numbers) + f(i-1, j-1)
    int n = a.length();
    vector<vector<int>> dp(n+1, vector<int>(n+1, 0));
    dp[0][0] = 0;
    for(int i=1; i<=n; i++) {
        dp[0][i] = (b[i-1] - 'a') + dp[0][i-1]; // Insert to make
        dp[i][0] = (a[i-1] - 'a') + dp[i-1][0]; // Delete to make
    }
    for(int i=1; i<=n; i++) {
        for(int j=1; j<=n; j++) {
            if(a[i-1] == b[i-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = std::min({
                    (b[j-1] - 'a') + dp[i][j-1], // Insert
                    (a[i-1] - 'a') + dp[i-1][j], // Delete
                    abs(b[j-1] - a[i-1]) + dp[i-1][j-1] // Substitute
                });
            }
        }
    }
    cout << dp[n][n] << endl;
    return 0;
}
