#include "Header.h"
using namespace std;
using namespace datans;

// default constructor for Experiment objects
Experiment::Experiment(std::string name, std::vector<std::string> v) : name_(name)
{
	for (auto iter = v.begin(); iter != v.end(); ++iter)
	{
		headings_.push_back(*iter);
	}
}

// rvalue move constructor for efficient moving of Experiment objects
Experiment::Experiment(Experiment&& e)
{
	// copy all types
	headings_ = e.headings_;
	measurementContainer_ = std::move(e.measurementContainer_);
	dataHeadings_ = e.dataHeadings_;
	name_ = e.name_;

	// now cleanup the original
	e.headings_.clear();
	e.dataHeadings_.clear();
	e.measurementContainer_.clear();
	e.name_.clear();
}

// error calculation for experiment objects
vector<double> Experiment::errorCalc()
{
	double rowCount{ 0 };
	int index{ 0 };

	vector<double> errors(headings_.size());

	// iterate over rows in experiment
	for (auto vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
	{
		rowCount++;
		// now iterate over the measurements within each row>
		for (auto meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
		{
			// we need to sum error / value for the COLUMN - so for each element of the row
			// we will add to the vector error in the same row format
			index = std::distance((*vec_iter).begin(), meas_it);
			if ((*meas_it)->getValue() != 0)
			{
				errors[index] += ((*meas_it)->getError() / fabs((*meas_it)->getValue()));
			}
		}
	}

	// divide by rowcount to get average error
	for_each(errors.begin(), errors.end(), [rowCount](double val)
	{
		val = val / rowCount;
	});
	return errors;
}

// add measurement based on a temporary measurement
std::shared_ptr<Measurement> datans::addMeasurement(std::vector<std::string> v)
{
	int vectorSize = v.size();
	std::shared_ptr<Measurement> ptr;	// base class pointer

	switch (vectorSize)
	{
	// size 4 = double type
	case 4:
		{
			// convert to relevant type
			double tempVal = stod(v[0]);
			double tempErr = stod(v[1]);
			double tempSyst = stod(v[2]);
			// create the measurement
			ptr.reset(new NumMeasure(tempVal, tempErr, tempSyst, v[3]));
			break;
		}
	// size 2 = string type
	case 2:
		{
			ptr.reset(new StringMeasure(v[0], v[1]));
			break;
		}	
	default:
		{
			throw "Invalid data type! Format should be (numerical value, error, systematic error, date) or (textual value, date)\n";
			break;
		}
		break;
	}
	return ptr;
}

// read experiment in either from file or from default data folder
void datans::readExperiment(std::string n, std::map<std::string, Experiment> &u, char readFlag)
{
	std::ifstream dataFile;
	std::string tempLine, tempLine2, buf;
	std::vector<std::string> tempHeadings, tempDataHeadings, tempMeasurement;
	std::vector<shared_ptr<Measurement>> rowMeasurement;
	std::string filePath;

	// reading from data folder
	if (readFlag == 'r')
	{
		dataFile.open(".//data//" + n);
		n.erase(n.end()-4, n.end());
	}

	// filepath parsed from openFileDialogue
	else if (readFlag == 'f')
	{
		dataFile.open(n);
		// reassign n to be just the name
		filePath = n;
		n = fileNameFromPath(n);
	}

	if (dataFile.is_open())
	{
		// read the first line which will be headings
		std::getline(dataFile, tempLine);
		std::stringstream ss(tempLine);
		while (ss >> buf)
		{
			tempHeadings.push_back(buf);
		}

		// create temporary object
		Experiment tempExp(n, tempHeadings);
		tempDataHeadings.resize(tempHeadings.size());
		static int counter{ 0 };

		// position should now be on the second line
		while (std::getline(dataFile, tempLine))
		{
			int colCount{ 0 };
			// we read the whole line, then each measurement is tab delineated, so split into n strings (each string is a measurement)
			// then create each measurement and push that back into a row. row is then pushed back into the measurement container
			std::stringstream ss(tempLine);
			while (std::getline(ss, tempLine2, '\t'))
			{
				std::stringstream ss2(tempLine2);
				while (ss2 >> buf)
				{
					tempMeasurement.push_back(buf);
				}			
				// only execute once to set dataheadings
				if (counter == 0)
				{
					if (tempMeasurement.size() == 4)
					{
						tempDataHeadings[colCount] = "Val    Err    SEerr  Date        ";
					}
					else
					{
						tempDataHeadings[colCount] = "Val                 Date        ";
					}
				}
				else
				// compare other measurements to the first to ensure same type down columns
				{
					if ((tempDataHeadings[colCount].length() < 13 && tempMeasurement.size() == 4) || (tempDataHeadings[colCount].length() > 13 && tempMeasurement.size() == 2))
					{
						perror("\nMeasurements in a column should be of the same type\n");
						return;
					}
				}
				colCount++;
				// try catch to find invalid input; no recovery (user must fix their file)
				try {
					rowMeasurement.push_back(std::move(addMeasurement(tempMeasurement)));
				}
				catch (const char* msg) {
					cerr << msg;
					cerr << "Check format of file " << n << " and try again." << endl;
					std::cin.clear();
					return;
				}
				tempMeasurement.clear();
			}
			tempExp.measurementContainer_.emplace_back(std::move(rowMeasurement));
		}
		tempHeadings.clear();

		// if file is from openFileDialogue, save the experiment locally to data folder
		if (readFlag == 'f')
		{
			// for checking file exists and can be wrote to
			int check = tempExp.saveExperiment('f');
			if (check != 1)
			{
				return;
			}
		}

		// move object to the map
		u[n] = std::move(tempExp);
		u[n].dataHeadings_ = tempDataHeadings;
		dataFile.close();
		std::cout << "Experiment " << n << " loaded successfully\n";
		return;
	}
	else
	{
		perror("Could not find or read file\n");
		return;
	}
}

// add experiment by hand
void datans::addExperiment(std::map<std::string, Experiment> &u)
{
	std::string tempName, tempStr, buf;
	std::vector<std::string> tempHeadings, tempDataHeadings;
	char existsFlag;

	std::cout << "Enter experiment name: ";
	std::getline(std::cin, tempName);

	// check if the experiment exists, if so can overwrite, if not exit
	std::map<std::string, Experiment>::iterator ptr;
	ptr = u.find(tempName);
	if (ptr != u.end())
	{
		cout << "Experiment with this name already exists! Overwrite? (Y/N): ";
		cin >> existsFlag;
		cin.ignore();
		if (tolower(existsFlag) == 'n')
		{
			cout << "Aborting adding of experiment.\n";
			return;
		}
	}

	std::cout << "\nEnter headings, space delimited - units in () i.e. Voltage(V): ";
	std::getline(std::cin, tempStr);
	std::stringstream ss(tempStr);
	while (ss >> buf)
	{
		tempHeadings.push_back(buf);
	}

	// create temporary object
	Experiment tempExp(tempName, tempHeadings);
	tempDataHeadings.resize(tempHeadings.size());

	char loopFlag('y');	// for do-while loop
	char isGood('y');	// for retrying of try-catch loop
	std::vector<string> tempMeasurement;
	std::vector<std::shared_ptr<Measurement>> rowMeasurement;
	static int counter{ 0 };

	do
	{
		for (unsigned int i = 0; i < tempHeadings.size(); ++i)
		{
			do {
				try {
					std::cout << "Enter data " << counter + 1 << " for " << tempHeadings[i] << " (space delimited).";
					std::cout << " Example formats: \"10 2 2 2016-05-03\"  or  \"Red 2016-05-03\" :\n";
					std::getline(std::cin, tempStr);
					std::stringstream ss(tempStr);
					while (ss >> buf)
					{
						tempMeasurement.push_back(buf);
					}
					// only need to execute this code once to set dataheadings
					if (counter == 0)
					{
						if (tempMeasurement.size() == 4)
						{
							tempDataHeadings[i] = "Value   Err   Systerr   Date";
						}
						else
						{
							tempDataHeadings[i] = "Value    Date";
						}
					}
					else
					// now we compare other measurements to the first to ensure same type down columns
					{
						if ((tempDataHeadings[i].length() < 13 && tempMeasurement.size() == 4) || (tempDataHeadings[i].length() > 13 && tempMeasurement.size() == 2))
						{
							throw("Measurements in a column should be of the same type");
						}
					}
					rowMeasurement.push_back(std::move(addMeasurement(tempMeasurement)));
					tempMeasurement.clear();
					isGood = 'y';	// flag set if no error is caught
				}
				// if the data is in an invalid format (throw from addMeasurement)
				catch (const char* msg) {
					cerr << msg << endl;
					std::cin.clear();
					isGood = 'n';	// set flag so that try-catch loop will retry
				}

				if (isGood != 'n')	// we only push back the row if data has been added successfully
				{
					tempExp.measurementContainer_.emplace_back(std::move(rowMeasurement));
					rowMeasurement.clear();
					break;
				}
				else
				{
					rowMeasurement.clear();
					tempMeasurement.clear();
				}
			} while (isGood = 'n');
		}
		counter++;
		std::cout << "Continue to add data? (Current rows: " << counter << ") (Y/N): ";
		std::cin >> loopFlag;
		std::cin.ignore();

	} while (loopFlag == 'Y' || loopFlag == 'y');

	// save the experiment locally
	int check = tempExp.saveExperiment('s');
	if (check != 1)
	{
		std::cout << "Could not save experiment to file.\n";
		return;
	}

	// move object to map
	u[tempName] = std::move(tempExp);
	u[tempName].dataHeadings_ = tempDataHeadings;
}

// print experiment to console
int Experiment::printExperiment()
{
	const char seperator = ' ';
	const int width = WIDTH;
	
	// extra spacing to offset the row numbers
	std::cout << endl;
	std::cout << "   ";
	// print out header numbers
	for (unsigned int i = 0; i < headings_.size(); ++i)
	{
		std::cout << std::left << std::setw(width) << setfill(seperator) << i+1;
	}
	std::cout << endl;

	std::cout << "   ";
	// print out the headings
	for_each(headings_.begin(), headings_.end(), [width, seperator](std::string h)
	{
		std::cout << std::left << std::setw(width) << setfill(seperator) << h;
	});
	std::cout << endl;

	std::cout << "   ";
	// print out column titles
	for_each(dataHeadings_.begin(), dataHeadings_.end(), [width, seperator](std::string h)
	{
		std::cout << std::left << std::setw(width) << setfill(seperator) << h;
	});
	std::cout << endl;

	// print measurements
	int rowcount{ 1 };
	// iterating over the container
	for (auto vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
	{
		std::cout << rowcount << "  ";
		// iterating over the measurements in the row
		for (vector<std::shared_ptr<Measurement>>::iterator meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
		{
			(*meas_it)->printInfo(seperator);
		}
		rowcount++;
		std::cout << endl;
	}

	// print out % error
	std::vector<double> errors = errorCalc();
	std::cout << "   ";
	for_each(errors.begin(), errors.end(), [width, seperator](double e)
	{
		std::cout << std::left << std::setw(width/4) << "% error: ";
		if (e != 0)
		{
			std::cout << std::setw(width-10) << e << std::setfill(seperator);
		}
		else
		{
			std::cout << std::setw(width-10) << "N/A" << std::setfill(seperator);
		}
	});
	std::cout << endl;
	return 1;
}

int Experiment::editExperiment()
{
	// print the experiment first for user reference
	cout << endl;
	printExperiment();

	vector<int> coordinates;	// [0] = row   [1] = col
	std::string tempCoord;
	std::string tempStr, buf;
	std::vector<std::string> tempMeasurement;
	char flag = ('y');
	const char seperator = ' ';
	const int width = WIDTH;
	int check{ 0 };
	int checkInputType{ 0 };

	do {
		cout << "\nEdit which measurement? (Row, column): ";
		std::getline(cin, tempCoord);
		// need to sanitise coordinate input - check for comma somewhere in string
		if (tempCoord.find(',') != std::string::npos)
		{
			// read string, split the string by comma and push back
			std::stringstream ss(tempCoord);
			while (ss.good())
			{
				std::string substr;
				getline(ss, substr, ',');
				// check input is numerical
				if (!substr.empty() && substr.find_first_not_of("123456789") == std::string::npos)
				{
					// check range is valid
					if (stoi(substr) - 1 < static_cast<int>(headings_.size()))
					{
						// -1 as user inputs starting from 1 for readability
						coordinates.push_back(stoi(substr) - 1);
					}
				}
				else
				{
					cout << "\nColumn out of range\n";
					return -1;
				}
			}
		}
		else
		{
			cout << "\nIncorrect row/column format.\n";
			return -1;
		}

		measurementContainer_[coordinates[0]][coordinates[1]]->printInfo(seperator);
		cout << endl << "\nEnter new data, space delimited (date will update automatically). Example format: \"20 23 3\"  or  \"Red\" : ";
		std::getline(std::cin, tempStr);
		std::stringstream ss2(tempStr);
		while (ss2 >> buf)
		{
			tempMeasurement.push_back(buf);
		}
		// check if operation completed successfully
		check = measurementContainer_[coordinates[0]][coordinates[1]]->updateInfo(tempMeasurement);
		tempMeasurement.clear();
		coordinates.clear();

		// we will only print the experiment if any data has changed; updateInfo returns -1 if invalid
		if (check == 1)
		{
			cout << "Measurement edited successfully" << endl;
			printExperiment();
		}
		cout << "\nContinue editing experiment? (Y/N): ";
		std::cin >> flag;
		std::cin.ignore();

	} while (tolower(flag) == 'y');
	
	// update the experiment file in /data
	check = saveExperiment('s');
	if (check == 1)
	{
		return 1;
	}
	else
	{
		cout << "No changes made to experiment " << name_ << ".\n";
		return -1;
	}
}

int Experiment::saveExperiment(char flag)
{
	char typeFlag;
	std::string filename;

	if (flag == 'e')
	{
		// check how we are saving the file
		cout << "Save to which format? [T]ext / [C]SV / [L]atex: ";
		cin >> typeFlag;
	
		switch (tolower(typeFlag))
		{
		case 't':
			filename = name_ + ".txt";
			break;
		case 'c':
			filename = name_ + ".csv";
			break;
		case 'l':
			filename = name_ + ".tex";
			break;
		}
	}

	else
	{
		filename = name_ + ".txt";
		typeFlag = 't';
	}

	if (isFileExist(filename) == true)
	{
		char saveFlag;
		std::cout << "File " << filename << " already exists. Overwrite? (Y/N): ";
		std::cin >> saveFlag;

		if (saveFlag == 'N' || saveFlag == 'n')
		{
			std::cout << "\nSave operation aborted.\n";
			return -1;
		}
	}

	// if file doesn't exist (or user chooses to overwrite)
	ofstream dataFile(".//data//" + filename);

	if (dataFile.is_open())
	{
		switch (typeFlag)
		{
		case 't':
			// save headings
			for (auto vec_iter = headings_.begin(); vec_iter != headings_.end(); ++vec_iter)
			{
				dataFile << (*vec_iter) << "\t";
			}
			dataFile << std::endl;

			// passed by reference so we can access the info properly
			// iterate over container rows
			for (auto& vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
			{
				// iterate over individual rows
				for (auto& meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
				{
					dataFile << (*meas_it)->saveInfo(typeFlag);	// flag determines how file is saved
				}
				dataFile << endl;
			}
			break;
		
		case 'c':
			// save headings
			for (auto vec_iter = headings_.begin(); vec_iter != headings_.end(); ++vec_iter)
			{
				dataFile << (*vec_iter) << "\t";
			}
			dataFile << std::endl;

			for (auto& vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
			{
				for (auto& meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
				{
					dataFile << (*meas_it)->saveInfo(typeFlag);
				}
				dataFile << endl;
			}
			break;
		
		case 'l':
			std::string buf;
			std::string tempStr;
			// latex formatting has some requirements for the document
			dataFile << "\\documentclass{article}" << endl;
			dataFile << "\\newcommand\\tab[1][0.8cm]{\\hspace*{#1}}" << endl;
			dataFile << "\\begin{document}" << endl;
			dataFile << "\\title{" << name_ << "}" << endl;
			dataFile << "\\begin{center}" << endl;
			
			dataFile << "\\begin{tabular}{";
			for (size_t i = 0; i < headings_.size(); ++i)
			{
				dataFile << "|c";
			}
			dataFile << "|}" << endl;
			dataFile << "\\hline" << endl;
				
			int counter{ 0 };	// to make sure & not at the end of line
			for (auto vec_iter = headings_.begin(); vec_iter != headings_.end(); ++vec_iter)
			{
				dataFile << (*vec_iter);
				if (counter != (distance(headings_.begin(), headings_.end()))-1)
				{
					dataFile << " & ";
				}
				++counter;
			}
			dataFile << "\\\\ [0.5ex]" << std::endl;
			dataFile << "\\hline" << endl;
			counter = 0;	// to make sure & not at the end of line
			for (auto vec_iter = dataHeadings_.begin(); vec_iter != dataHeadings_.end(); ++vec_iter)
			{
				// open stringstream on (*vec_iter)
				istringstream ss(*vec_iter);
				while (ss >> buf)
				{
					dataFile << buf;
					dataFile << "\\tab ";
				}
				
				if (counter != (distance(dataHeadings_.begin(), dataHeadings_.end())) - 1)
				{
					dataFile << " & ";
				}
				++counter;
			}

			dataFile << "\\\\ [0.5ex]" << std::endl;
			dataFile << "\\hline\\hline" << endl;			
			for (auto& vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
			{
				counter = 0;	// to make sure & not at the end of line
				for (auto& meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
				{
					dataFile << (*meas_it)->saveInfo(typeFlag);
					if (counter != (distance((*vec_iter).begin(), (*vec_iter).end())) - 1)
					{
						dataFile << " & ";
					}
					++counter;
				}
				dataFile << "\\\\" << endl;
				dataFile << "\\hline" << endl;
			}
			dataFile << "\\end{tabular}" << endl;
			dataFile << "\\end{center}" << endl;
			dataFile << "\\end{document}" << endl;
			break;
		}

		dataFile.close();
		cout << "File " << filename << " saved successfully" << endl;
		return 1;
	}
	else
	{
		perror("Could not open file\n");
		return -1;
	}
}

int Experiment::deleteExperiment(std::map<std::string, Experiment> &u)
{
	char deleteFlag;
	std::cout << "Confirm deletion of experiment " << (*this).name_ << " (Y/N): " << endl;
	cin >> deleteFlag;
	if (tolower(deleteFlag) == 'y')
	{
		// create a string to filepath then pass this as a const char * to remove
		std::string filePath = ".//data//" + name_ + ".txt";
		if (remove(filePath.c_str()) != 0)
		{
			// print error file not found
			perror("Error deleting data file");
			return -1;
		}
		else
		{
			// delete the pair from the map
			u.erase(name_);
			std::cout << "Experiment deleted successfully" << endl;
			return 1;
		}
	}
	else
	{
		std::cout << "Aborting deletion" << endl;
		return -1;
	}
}