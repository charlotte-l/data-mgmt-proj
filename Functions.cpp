#include "Header.h"
using namespace std;
using namespace datans;

std::vector<std::string> readDir()
{
	DIR *dir;
	struct dirent *ent;
	vector<std::string> fileList;
	if ((dir = opendir("./data")))
	{
		// print all files and directories in data
		while (ent = readdir(dir))
		{
			// ignore '.' and '..' directory references
			if ((*ent->d_name) != '.' && (*ent->d_name) != '..')
			{
				fileList.push_back(ent->d_name);
			}
		}
		closedir(dir);
		return fileList;
	}
	else
	{
		/* could not open directory */
		perror("Could not find data directory. Creating new directory...");
		return fileList;
	}
}

// function to test whether file already exists
bool isFileExist(std::string &n)
{
	std::ifstream infile(".//data//" + n + ".txt");
	return infile.good();
}

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
	name_.assign(e.name_, 0, name_.length());

	e.headings_.clear();
	e.dataHeadings_.clear();
	e.measurementContainer_.clear();
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
				errors[index] += ((*meas_it)->getError() / (*meas_it)->getValue());
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
	}
	return ptr;
}

int datans::readExperiment(std::string &n, std::map<std::string, Experiment> &u, char readFlag)
{
	std::ifstream dataFile;
	std::string tempLine, tempLine2, buf, filePath;
	std::vector<std::string> tempHeadings, tempDataHeadings, tempMeasurement;
	std::vector<shared_ptr<Measurement>> rowMeasurement;

	if (readFlag == 'r')
		dataFile.open(".//data//" + n);

	else if (readFlag == 'f')
	{
		std::cout << "Enter filepath to file location (i.e. C://dataFiles// - N.B double slash!): ";
		cin.ignore();
		getline(cin, filePath);
		dataFile.open(filePath + n);
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

		n.erase(n.end() - 4, n.end());
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
						tempDataHeadings[colCount] = "Val  Err  SEerr  Date";
					}
					else
					{
						tempDataHeadings[colCount] = "Val  Date";
					}
				}
				else
				// now we compare other measurements to the first to ensure same type down columns
				{
					if ((tempDataHeadings[colCount].length() < 13 && tempMeasurement.size() == 4) || (tempDataHeadings[colCount].length() > 13 && tempMeasurement.size() == 2))
					{
						perror("Measurements in a column should be of the same type");
						return -1;
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
				std::cout << "Could not save experiment to file.\n";
		}

		u.insert(std::make_pair(n, std::move(tempExp)));
		u[n].dataHeadings_ = tempDataHeadings;
		dataFile.close();
		return 1;
	}
	else
		std::cout << "Could not find or read file " << n << endl;
	return -1;
}

int datans::addExperiment(std::map<std::string, Experiment> &u)
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
			return -1;
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

	char flag('y');	// for do-while loop
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
						tempDataHeadings[i] = "Value  Err  Systerr  Date";
					}
					else
					{
						tempDataHeadings[i] = "Value  Date";
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
		std::cin >> flag;
		std::cin.ignore();

	} while (flag == 'Y' || flag == 'y');

	// save the experiment locally
	int check = tempExp.saveExperiment();
	if (check != 1)
	{
		std::cout << "Could not save experiment to file.\n";
		return -1;
	}

	u.insert(std::make_pair(tempName, std::move(tempExp)));
	u[tempName].dataHeadings_ = tempDataHeadings;
	return 1;
}

int datans::printExperiment(std::string &n, std::map<std::string, Experiment> u)
{
	std::map<std::string, Experiment>::iterator ptr;
	ptr = u.find(n);
	if (ptr != u.end())
	{
		for (unsigned int i = 0; i < (ptr->second).headings_.size(); ++i)
		{
			std::cout << "  " << std::left << std::setw(20) << i+1;
		}
		std::cout << endl;

		// print out the headings first
		for_each((ptr->second).headings_.begin(), (ptr->second).headings_.end(), [](std::string h)
		{
			std::cout << "  " << std::left << std::setw(20) << h;
		});
		std::cout << endl;

		// now print out under each heading a column title based on the first element
		for_each((ptr->second).dataHeadings_.begin(), (ptr->second).dataHeadings_.end(), [](std::string h)
		{
			std::cout << "  " << std::left << std::setw(20) << h << std::setw(10) << "\t";
		});
		std::cout << endl;

		int rowcount{ 1 };
		for (auto vec_iter = (ptr->second).measurementContainer_.begin(); vec_iter != (ptr->second).measurementContainer_.end(); ++vec_iter)
		{
			std::cout << rowcount << "  ";
			// iterating over the row
			for (vector<std::shared_ptr<Measurement>>::iterator meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
			{
				(*meas_it)->printInfo();
				std::cout << "\t";
			}
			rowcount++;
			std::cout << endl;
		}

		// finally print out % error
		std::vector<double> errors = (ptr->second).errorCalc();
		for_each(errors.begin(), errors.end(), [](double e)
		{
			std::cout << std::left << std::setw(5) << "% error: " << std::setw(5);
			if (e != 0)
			{
				std::cout << e << "\t";
			}
			else
			{
				std::cout << "N/A" << std::setw(5) << "\t";
			}
		});
		std::cout << endl;
		return 1;
	}
	else
	{
		return -1;
	}
}

int datans::editExperiment(std::string &n, std::map<std::string, Experiment> u)
{
	cout << endl;
	char flag = ('y');
	int check = printExperiment(n, u);
	if (check != 1)
	{
		return -1;
	}

	std::map<std::string, Experiment>::iterator ptr;
	ptr = u.find(n);

	vector<int> coordinates;	// [0] = row   [1] = col
	std::string tempCoord;
	std::string tempStr, buf;
	std::vector<std::string> tempMeasurement;

	do {
		cout << "\nEdit which measurement? (Row, column): ";
		std::getline(cin, tempCoord);

		std::stringstream ss(tempCoord);
		while (ss.good())
		{
			std::string substr;
			getline(ss, substr, ',');
			coordinates.push_back(stoi(substr)-1);
		}

		ptr->second.measurementContainer_[coordinates[0]][coordinates[1]]->printInfo();
		cout << endl << "\nEnter new data, space delimited (date will update automatically): ";

		std::getline(std::cin, tempStr);
		std::stringstream ss2(tempStr);
		while (ss2 >> buf)
		{
			tempMeasurement.push_back(buf);
		}

		check = ptr->second.measurementContainer_[coordinates[0]][coordinates[1]]->updateInfo(tempMeasurement);
		tempMeasurement.clear();
		coordinates.clear();

		// we will only print the experiment if any data has changed; updateInfo returns -1 if invalid
		if (check = 1)
		{
			cout << endl;
			printExperiment(n, u);
		}
		cout << "\nContinue editing experiment? (Y/N): ";
		std::cin >> flag;
		std::cin.ignore();

	} while (tolower(flag) == 'y');

	return 1;

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
			std::cout << "Save operation aborted.\n";
			return -1;
		}
	}

	// if file doesn't exist (or user chooses to overwrite)
	ofstream dataFile;
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
				dataFile << (*meas_it)->saveInfo();
			}
			dataFile << endl;
		}
		dataFile.close();
		return 1;
	}
	else
	{
		std::cout << "Could not find experiment\n";
		return -1;
	}
}

int datans::deleteExperiment(std::string &n, std::map<std::string, Experiment> &u)
{
	char deleteFlag;
	std::cout << "Confirm deletion of experiment " << n << " (Y/N): " << endl;
	cin >> deleteFlag;
	if (tolower(deleteFlag) == 'y')
	{
		// create a string to filepath then pass this as a const char * to remove
		std::string filePath = ".//data//" + n + ".txt";
		if (remove(filePath.c_str()) != 0)
		{
			// print error file not found
			perror("Error deleting data file for experiment");
			return -1;
		}
		else
		{
			// delete the pair from the map
			u.erase(n);
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