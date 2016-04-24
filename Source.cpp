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
		readExperiment((*iter), user);
		cout << "Experiment " << (*iter) << " loaded." << endl;
	}

	bool menu{ true };
	char menuFlag;
	std::string nameFlag;
	std::string fileName;
	char addFlag;
	char deleteFlag;
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
		case '1':
			cout << "Select an experiment to view: " << endl;
			// print out the current list
			for (std::map<std::string, experiment>::iterator it = user.begin(); it != user.end(); ++it)
			{
				std::cout << it->first << endl;
			}
			cin >> nameFlag;
			printExperiment(nameFlag, user);
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
				readExperiment(fileName, user);
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
				cout << "Confirm deletion of experiment " << fileName << "(Y/N): " << endl;
				cin >> deleteFlag;
				if (tolower(deleteFlag) == 'y')
				{
					deleteExperiment(fileName, user);
					break;
				}
				else
				{
					cout << "Aborting deletion" << endl;
					break;
				}
			}
			else
			{
				cout << "Could not find experiment" << endl;
				break;
			}

		case '4':
			menu = false;
			break;

		default: 
			cout << "Command not recognised" << endl;
			break;
		}

	} while (menu == true);

	return 0;
}