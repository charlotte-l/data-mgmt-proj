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
	
	// initialise map and filelist
	std::map<std::string, Experiment> user;
	std::vector<std::string> fileList = readDir();

	if (fileList.empty() != 1)
	{
		cout << "Loading previous experiments..." << endl;
	}

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

	// user menu
	cout << "\nWelcome to DataManager. Select an option:" << endl;
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
			printExperimentList(user);
			getline(cin, fileName);
			// find the object - if found, print it's info
			ptr = user.find(fileName);
			if (ptr != user.end())
			{
				ptr->second.printExperiment();
				break;
			}
			else
			{
				cout << "Cannot find experiment " << fileName << endl;
				break;
			}
			break;
			
		case '2':
			cout << "Add experiment [m]anually or from [f]ile: ";
			cin >> addType; cin.ignore();
			switch (tolower(addType))
			{
			case 'm':
				addExperiment(user);
				break;

			case 'f':
				cout << "Select a file: " << endl;
				// call openfiledialogue, parse it's path then open the file
				filePath = openFileDialogue();
				fileName = fileNameFromPath(filePath);
				
				if (std::find(fileList.begin(), fileList.end(), fileName + ".txt") != fileList.end())		// check if a file with that name already exists
				{
					cout << "Experiment with this name already exists!" << endl;
					break;
				}
				// flag f to read from file
				readExperiment(filePath, user, 'f');
				break;
			default:
				cout << "Command not recognised.\n";
				cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				break;
			}
			break;

		case '3':
			cout << "Select an experiment to edit (options: ";
			printExperimentList(user);
			getline(cin, fileName);
			// check if object exists, if so move to editExperiment routine
			ptr = user.find(fileName);
			if (ptr != user.end())
			{
				ptr->second.editExperiment();
				break;
			}
			else
			{
				cout << "Cannot find experiment " << fileName << endl;
				break;
			}
			break;

		case '4':
			cout << "Select an experiment to export (options: ";
			printExperimentList(user);
			getline(cin, fileName);
			// check if object exists, if so move to saveExperiment routine
			ptr = user.find(fileName);
			if (ptr != user.end())
			{
				ptr->second.saveExperiment('e');
				break;
			}
			else
			{
				cout << "Cannot find experiment " << fileName << endl;
				break;
			}
			break;

		case '5':
			cout << "Enter name of experiment to delete (options: ";
			printExperimentList(user);
			getline(cin, fileName);
			ptr = user.find(fileName);
			// check if object exists, if so move to deleteExperiment routine
			if (ptr != user.end())
			{
				ptr->second.deleteExperiment(user);
				break;
			}
			else
			{
				cout << "Cannot find experiment " << fileName << endl;
				break;
			}
			break;

		case '6':
			// set flag to false to exit do-while loop
			menu = false;
			break;

		default: 
			cout << "Command not recognised" << endl;
			cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cout << endl << endl << endl << endl;
			break;		
		}
		cout << endl << endl;
	} while (menu == true);
	
	return 0;
}