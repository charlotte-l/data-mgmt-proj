#include "Header.h"

using namespace std;
using namespace datans;

int main()
{
	vector<experiment> user;

	measurement *testMeasure = new numMeasure;
	testMeasure->printInfo();

	addExperiment(user);
	delete testMeasure;

	return 0;
}