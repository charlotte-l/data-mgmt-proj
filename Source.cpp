#include "Header.h"

using namespace std;
using namespace datans;

int main()
{
	/******************************************************************
	program initiation process:
	1. create an empty map of experiments for current user
	2. populate this map by calls to readExperiment for the folder
	   experiments containing all experiment .txt files
	3. display user menu with options to display experiments or add a
	   new experiment
	******************************************************************/
	
	std::map<std::string, experiment> user;
	std::vector<std::string> filelist = readDir();

	cout << "Loading previous experiments..." << endl;

	// load all existing experiments into memory
	for (auto iter = filelist.begin(); iter != filelist.end(); ++iter)
	{
		readExperiment((*iter), user, 'r');
		cout << "Experiment " << (*iter) << " loaded." << endl;
	}

	bool menu{ true };
	char menuFlag;
	std::string nameFlag;
	std::string fileName;
	char addFlag;
	std::map<std::string, experiment>::iterator ptr;

	// user menu
	cout << "Welcome to DataManager. Select an option:" << endl;
	do
	{
		cout << endl << "[1]. Display experiment" << endl;
		cout << "[2]. Add experiment" << endl;
		cout << "[3]. Delete experiment " << endl;
		cout << "[4]. Exit" << endl;

		cout << "Option: "; cin >> menuFlag; cout << endl;

		switch (menuFlag)
		{
		case '1':		// printing experiment to console
			cout << "Select an experiment to view (options: ";
			// print out the current list of experiments in memory
			for (std::map<std::string, experiment>::iterator it = user.begin(); it != user.end(); ++it)
			{
				std::cout << it->first << ", ";
			}
			cout << "\b\b): ";
			cin >> nameFlag;
			cout << endl;
			// try-catch to catch improper input
			try {
				printExperiment(nameFlag, user);
			}
			catch (const char* msg) {
				cerr << msg << endl;
				std::cin.clear();
			}
			break;

		case '2':
			cout << "Add experiment [m]anually or from [f]ile: ";
			cin >> addFlag;
			switch (tolower(addFlag))
			{
			case 'm':
				addExperiment(user);
				break;
			case 'f':
				cout << "Enter filename: ";
				cin >> fileName;
				readExperiment(fileName, user, 'f');
				break;
			default:
				cout << "Input invalid" << endl;
				break;
			}
			break;

		case '3':
			cout << "Enter name of experiment to delete: ";
			cin >> fileName;
			ptr = user.find(fileName);
			// check the experiment does exist
			if (ptr != user.end())
			{
				deleteExperiment(fileName, user);
				break;
			}
			else
			{
				cout << "Cannot find experiment" << fileName << endl;
				break;
			}

		case '4':
			menu = false;
			break;

		default: 
			cout << "Command not recognised" << endl;
			cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			break;
		}

	} while (menu == true);

	return 0;
}