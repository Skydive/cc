#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {
	bool end = false;
	while(!end) {
		char buf[256];
		cin.getline(buf, sizeof(buf));
		int num = atoi(buf);
		if(num == 42) {
			break;
		} else {
			cout << num << endl;
		}
	}
	return 0;
}
