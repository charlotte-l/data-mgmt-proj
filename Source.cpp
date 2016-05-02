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
		readExperiment((*iter), user, 'r');
	}
	
	bool menu{ true };
	char menuOption;
	char addType;
	std::string fileName;
	std::string filePath;
	std::map<std::string, Experiment>::iterator ptr;

	cout << "Welcome to DataManager. Select an option:" << endl;
	do
	{
		cout << endl;
		cout << "[1]. Display experiment" << endl;
		cout << "[2]. Add experiment" << endl;
		cout << "[3]. Edit experiment" << endl;
		cout << "[4]. Export experiment" << endl;
		cout << "[5]. Delete experiment " << endl;
		cout << "[6]. Exit" << endl;

		cout << "Option: "; cin >> menuOption; cin.ignore(); cout << endl;
		switch (menuOption)
		{
		case '1':
			cout << "Select an experiment to view (options: ";
			// print out the current list of experiments in memory
			printExperimentList(user);
			getline(cin, fileName);
			ptr = user.find(fileName);
			if (ptr != user.end())
			{
				ptr->second.printExperiment();
				break;
			}
			else
			{
				cout << "Cannot find experiment" << fileName << endl;
				break;
			}
			break;
			
		case '2':
			cout << "Add experiment [m]anually or from [f]ile: ";
			cin >> addType;
			cin.ignore();
			switch (tolower(addType))
			{
			case 'm':
				addExperiment(user);
				break;

			case 'f':
				cout << "Select a file: " << endl;
				filePath = openFileDialogue();
				fileName = fileNameFromPath(filePath);
				
				if (std::find(fileList.begin(), fileList.end(), fileName + ".txt") != fileList.end())		// check if a file with that name already exists
				{
					cout << "Experiment with this name already exists!" << endl;
					break;
				}
				readExperiment(filePath, user, 'f');
				break;
			}
			break;

		case '3':
			cout << "Select an experiment to edit (options: ";
			printExperimentList(user);
			getline(cin, fileName);
			ptr = user.find(fileName);
			if (ptr != user.end())
			{
				ptr->second.editExperiment();
				break;
			}
			else
			{
				cout << "Cannot find experiment" << fileName << endl;
				break;
			}
			break;

		case '4':
			cout << "Select an experiment to export (options: ";
			printExperimentList(user);
			getline(cin, fileName);
			ptr = user.find(fileName);
			if (ptr != user.end())
			{
				ptr->second.saveExperiment();
				break;
			}
			else
			{
				cout << "Cannot find experiment" << fileName << endl;
				break;
			}
			break;

		case '5':
			cout << "Enter name of experiment to delete (options: ";
			printExperimentList(user);
			getline(cin, fileName);
			ptr = user.find(fileName);
			// check the experiment does exist
			if (ptr != user.end())
			{
				ptr->second.deleteExperiment(user);
				break;
			}
			else
			{
				cout << "Cannot find experiment" << fileName << endl;
				break;
			}
			break;

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