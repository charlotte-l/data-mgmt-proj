#include "Header.h"
using namespace std;
using namespace datans;

Experiment::Experiment(std::string name, std::vector<std::string> v) : name_(name)
{
	for (auto iter = v.begin(); iter != v.end(); ++iter)
	{
		headings_.push_back(*iter);
	}
}

// rvalue move constructor for efficient moving of experiment objects
Experiment::Experiment(Experiment&& e)
{
	headings_ = e.headings_;
	measurementContainer_ = std::move(e.measurementContainer_);
	dataHeadings_ = e.dataHeadings_;
	name_ = e.name_;
	//name_.assign(e.name_, 0, name_.length());

	e.headings_.clear();
	e.dataHeadings_.clear();
	e.measurementContainer_.clear();
	e.name_.clear();
}

vector<double> Experiment::errorCalc()
{
	double rowCount{ 0 };
	int index{ 0 };

	vector<double> errors(headings_.size());

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

	for_each(errors.begin(), errors.end(), [rowCount](double val)
	{
		val = val / rowCount;
	});
	return errors;
}

std::shared_ptr<Measurement> datans::addMeasurement(std::vector<std::string> v)
{
	int vectorSize = v.size();
	std::shared_ptr<Measurement> ptr;

	switch (vectorSize)
	{
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

void datans::readExperiment(std::string n, std::map<std::string, Experiment> &u, char readFlag)
{
	std::ifstream dataFile;
	std::string tempLine, tempLine2, buf;
	std::vector<std::string> tempHeadings, tempDataHeadings, tempMeasurement;
	std::vector<shared_ptr<Measurement>> rowMeasurement;

	if (readFlag == 'r')
	{
		dataFile.open(".//data//" + n);
		n.erase(n.end()-4, n.end());
	}

	else if (readFlag == 'f')
	{
		dataFile.open(n);
		// reassign n
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
			while (getline(ss, tempLine2, '\t'))
			{
				std::stringstream ss2(tempLine2);
				while (ss2 >> buf)
				{
					tempMeasurement.push_back(buf);
				}
				
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
				rowMeasurement.push_back(std::move(addMeasurement(tempMeasurement)));
				tempMeasurement.clear();
			}
			tempExp.measurementContainer_.emplace_back(std::move(rowMeasurement));
		}
		tempHeadings.clear();

		// if flag is f, save the experiment locally
		if (readFlag == 'f')
		{
			int check = tempExp.saveExperiment();
			if (check != 1)
			{
				return;
			}
		}

		u.insert(std::make_pair(n, std::move(tempExp)));
		u[n].dataHeadings_ = tempDataHeadings;
		dataFile.close();
		cout << "Experiment " << n << " loaded successfully\n";
		return;
	}
	else
	{
		perror("Could not find or read file\n");
		return;
	}
}

void datans::addExperiment(std::map<std::string, Experiment> &u)
{
	std::string tempName, tempStr, buf;
	std::vector<std::string> tempHeadings, tempDataHeadings;
	char existsFlag;

	std::cout << "Enter experiment name: ";
	std::getline(std::cin, tempName);

	std::map<std::string, Experiment>::iterator ptr;
	ptr = u.find(tempName);
	if (ptr != u.end())
	{
		cout << "Experiment with this name already exists! Overwrite? (Y/N): ";
		cin >> existsFlag;
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

	Experiment tempExp(tempName, tempHeadings);
	tempDataHeadings.resize(tempHeadings.size());

	char loopFlag('y');	// for do-while loop
	std::vector<string> tempMeasurement;
	std::vector<std::shared_ptr<Measurement>> rowMeasurement;
	static int counter{ 0 };

	do
	{
		for (unsigned int i = 0; i < tempHeadings.size(); ++i)
		{
			try {
				std::cout << "Enter data " << counter + 1 << " for " << tempHeadings[i] << " (space delimited):\n";
				std::getline(std::cin, tempStr);
				std::stringstream ss(tempStr);
				while (ss >> buf)
				{
					tempMeasurement.push_back(buf);
				}
				// only need to execute this code once
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
			}
			// if the data is in an invalid format (throw from addMeasurement)
			catch (const char* msg) {
				cerr << msg << endl;
				std::cin.clear();
				break;
			}
			tempExp.measurementContainer_.emplace_back(std::move(rowMeasurement));
			rowMeasurement.clear();
			counter++;
		}
		
		std::cout << "Continue to add data? (Current rows: " << counter << ") (Y/N): ";
		std::cin >> loopFlag;
		std::cin.ignore();

	} while (loopFlag == 'Y' || loopFlag == 'y');

	// save the experiment locally
	int check = tempExp.saveExperiment();
	if (check != 1)
	{
		std::cout << "Could not save experiment to file.\n";
		return;
	}

	u.insert(std::make_pair(tempName, std::move(tempExp)));
	u[tempName].dataHeadings_ = tempDataHeadings;
}

int Experiment::printExperiment()
{
	const char seperator = ' ';
	const int width = WIDTH;
	
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
	for (auto vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
	{
		std::cout << rowcount << "  ";
		// iterating over the row
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
	cout << endl;
	printExperiment();

	vector<int> coordinates;	// [0] = row   [1] = col
	std::string tempCoord;
	std::string tempStr, buf;
	std::vector<std::string> tempMeasurement;
	char flag = ('y');
	const char seperator = ' ';
	const int width = WIDTH;
	int check;

	do {
		cout << "\nEdit which measurement? (Row, column): ";
		std::getline(cin, tempCoord);
		// need to sanitise coordinate input
		if (tempCoord.find(',') != std::string::npos)
		{
			std::stringstream ss(tempCoord);
			while (ss.good())
			{
				std::string substr;
				getline(ss, substr, ',');

				if (stoi(substr) - 1 < static_cast<int>(headings_.size()))
				{
					coordinates.push_back(stoi(substr) - 1);
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
		cout << endl << "\nEnter new data, space delimited (date will update automatically): ";

		std::getline(std::cin, tempStr);
		std::stringstream ss2(tempStr);
		while (ss2 >> buf)
		{
			tempMeasurement.push_back(buf);
		}

		check = measurementContainer_[coordinates[0]][coordinates[1]]->updateInfo(tempMeasurement);
		tempMeasurement.clear();
		coordinates.clear();

		// we will only print the experiment if any data has changed; updateInfo returns -1 if invalid
		if (check = 1)
		{
			cout << endl;
			printExperiment();
		}
		cout << "\nContinue editing experiment? (Y/N): ";
		std::cin >> flag;
		std::cin.ignore();

	} while (tolower(flag) == 'y');
	check = saveExperiment();
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

int Experiment::saveExperiment()
{
	std::string filename_ = name_ + ".txt";
	if (isFileExist(name_) == true)
	{
		char flag;
		std::cout << "File " << filename_ << " already exists. Overwrite? (Y/N): ";
		std::cin >> flag;

		if (flag == 'N' || flag == 'n')
		{
			std::cout << "\nSave operation aborted.\n";
			return -1;
		}
	}

	// if file doesn't exist (or user chooses to overwrite)
	ofstream dataFile;
	char tempflag{ 't' };

	dataFile.open(".//data//" + filename_);
	if (dataFile.is_open())
	{
		for (auto vec_iter = headings_.begin(); vec_iter != headings_.end(); ++vec_iter)
		{
			dataFile << (*vec_iter) << "\t";
		}
		dataFile << std::endl;

		// passed by reference so we can access the info properly
		for (auto& vec_iter = measurementContainer_.begin(); vec_iter != measurementContainer_.end(); ++vec_iter)
		{
			for (auto& meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
			{
				dataFile << (*meas_it)->saveInfo(tempflag);
			}
			dataFile << endl;
		}
		dataFile.close();
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