#include <iostream>
#include <cstdlib>

using namespace std;

#include <cmath>

int main(int argc, char** argv) {
	bool end = false;
	char buf[256];
	cin.getline(buf, sizeof(buf));
    int num_tests = atoi(buf);

    for(int i=0; i<num_tests; i++) {
        int m, n;
        cin >> m;
        cin >> n;
        for(int i=m; i<=n; i++) {
            bool isprime = true;
            int mid = (int)floor(sqrt((float)i));
            if(i == 1) { // CHECK CASE OF 1
                isprime = false;
            } else {
                for(int j=2; j<=mid; j++) {
                    if(i % j == 0) {
                        isprime = false;
                        break;
                    }
                }
            }
            if(isprime) {
                cout << i << endl;
            }
        }
        cout << endl;
    }
    return 0;
}
