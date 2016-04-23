#include "Header.h"

using namespace std;
using namespace datans;

int main()
{
	vector<experiment> user;

	std::map<std::string, experiment> user2;

	measurement *testMeasure = new numMeasure;
	testMeasure->printInfo();

	
	delete testMeasure;

	return 0;
}