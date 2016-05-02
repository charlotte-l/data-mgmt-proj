#include "Header.h"

using namespace std;
using namespace datans;

int main()
{
	// initial call to resize console window (it's gonna be big)
	setConsoleSize(750, 450);

	/******************************************************************
	program initiation process:
	1. create an empty map of experiments for current user
	2. populate this map by calls to readExperiment for the folder
	   experiments containing all experiment .txt files
	3. display user menu with options to display experiments or add a
	   new experiment
	******************************************************************/
	
	std::map<std::string, Experiment> user;
	std::vector<std::string> fileList = readDir();

	cout << "Loading previous experiments..." << endl;

	// load all existing experiments into memory
	for (auto iter = fileList.begin(); iter != fileList.end(); ++iter)
	{
		int check = readExperiment((*iter), user, 'r');
		if (check == 1)
			cout << "Experiment " << (*iter) << " loaded." << endl;
	}

	bool menu{ true };
	char menuFlag;
	char addFlag;
	int check;
	std::string nameFlag;
	std::string filePath;
	std::map<std::string, Experiment>::iterator ptr;
	
	// user menu
	cout << "Welcome to DataManager. Select an option:" << endl;
	do
	{
		cout << endl << "[1]. Display experiment" << endl;
		cout << "[2]. Add experiment" << endl;
		cout << "[3]. Edit experiment" << endl;
		cout << "[4]. Export experiment" << endl;
		cout << "[5]. Delete experiment " << endl;
		cout << "[6]. Exit" << endl;

		cout << "Option: "; cin >> menuFlag; cin.ignore(); cout << endl;
		switch (menuFlag)
		{
		case '1':		// printing experiment to console
			cout << "Select an experiment to view (options: ";
			// print out the current list of experiments in memory
			for (std::map<std::string, Experiment>::iterator it = user.begin(); it != user.end(); ++it)
			{
				std::cout << it->first << ", ";
			}
			cout << "\b\b): ";
			getline(cin, nameFlag);
			cout << endl;
			
			// pseudo exception handling for invalid input
			check = printExperiment(nameFlag, user);
			if (check != 1)
			{
				cout << "Experiment " << nameFlag << " could not be found" << endl;
			}
			break;
			
		case '2':
			cout << "Add experiment [m]anually or from [f]ile: ";
			cin >> addFlag;
			cin.ignore();
			switch (tolower(addFlag))
			{
			case 'm':
				addExperiment(user);
				break;
			case 'f':
				cout << "Select a file: " << endl;
				// check if a file with that name already exists
				filePath = openFileDialogue();
				nameFlag = fileNameFromPath(filePath);
				if (std::find(fileList.begin(), fileList.end(), nameFlag) != fileList.end())
				{
					cout << "Experiment with this name already exists!" << endl;
					break;
				}

				check = readExperiment(filePath, user, 'f');
				if (check == 1)
				{
					cout << "Experiment " << nameFlag << " loaded successfully." << endl;
				}
				break;
			default:
				cout << "Input invalid" << endl;
				break;
			}
			break;

		case '3':
			cout << "Select an experiment to edit (options: ";
			// print out the current list of experiments in memory
			for (std::map<std::string, Experiment>::iterator it = user.begin(); it != user.end(); ++it)
			{
				std::cout << it->first << ", ";
			}
			cout << "\b\b): ";
			getline(cin, nameFlag);

			check = editExperiment(nameFlag, user);
			if (check != 1)
			{
				cout << "Experiment " << nameFlag << " could not be found" << endl;
			}
			break;

		case '4':
			cout << "Select an experiment to export (options: ";
			// print out the current list of experiments in memory
			for (std::map<std::string, Experiment>::iterator it = user.begin(); it != user.end(); ++it)
			{
				std::cout << it->first << ", ";
			}
			cout << "\b\b): ";
			getline(cin, nameFlag);
			ptr = user.find(nameFlag);
			if (ptr != user.end())
			{
				ptr->second.saveExperiment();
				break;
			}
			else
			{
				cout << "Cannot find experiment" << nameFlag << endl;
				break;
			}

		case '5':
			cout << "Enter name of experiment to delete (options: ";
			for (std::map<std::string, Experiment>::iterator it = user.begin(); it != user.end(); ++it)
			{
				std::cout << it->first << ", ";
			}
			cout << "\b\b): ";
			cin >> nameFlag;
			ptr = user.find(nameFlag);
			// check the experiment does exist
			if (ptr != user.end())
			{
				deleteExperiment(nameFlag, user);
				break;
			}
			else
			{
				cout << "Cannot find experiment" << nameFlag << endl;
				break;
			}

		case '6':
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