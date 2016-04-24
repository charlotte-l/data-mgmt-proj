#include "Header.h"
using namespace std;
using namespace datans;

// function definitions go here

std::vector<std::string> readDir()
{
	DIR *dir;
	struct dirent *ent;
	vector<std::string> filelist;
	if ((dir = opendir("./data")))
	{
		// print all files and directories in data
		while (ent = readdir(dir))
		{
			filelist.push_back(ent->d_name);
		}
		closedir(dir);
		return filelist;
	}
	else
	{
		/* could not open directory */
		perror("Could not open directory");
		return filelist;
	}
}

void stringMeasure::printInfo()
{
	cout << value << "  " << date << endl;
}

void numMeasure::printInfo()
{
	cout << value << "  " << error << "  " << systError << "  " << "  " << date << endl;
}

// function to test whether file already exists
bool is_file_exist(std::string &n)
{
	std::ifstream infile(n);
	return infile.good();
}

measurement* datans::addMeasurement(std::vector<std::string> v)
{
	int vectorSize = v.size();
	if (vectorSize == 4)
	{
		// convert to relevant type
		double tempval = stod(v[0]);
		double temperr = stod(v[1]);
		double tempsyst = stod(v[2]);
		// create the measurement
		measurement *ptr;
		ptr = new numMeasure(tempval, temperr, tempsyst, v[3]);
		return ptr;
	}

	else if (vectorSize == 2)
	{
		measurement *ptr;
		ptr = new stringMeasure(v[0], v[1]);
		return ptr;
	}
	else
	{
		throw "Invalid data type! Format should be (numerical value, error, systematic error, date) or (textual value, date)\n";
	}
}

void datans::printExperiment(std::string n, std::map<std::string, experiment> u)
{
	// iterator so we can iterate through every measurement in the experiment
	vector<measurement>::iterator it;

}

// function to save experiments to a file
void experiment::saveExperiment(std::string n, std::map<std::string, experiment> u)
{
	std::string filename = n + ".txt";		// append .txt to the experiment name
	
	// check if file exists; if so, check if user wants to overwrite or not
	if (is_file_exist(n) == true)
	{
		char flag;
		cout << n << "already exists. Overwrite? (Y/N): ";
		cin >> flag;

		if (flag == 'N' || flag == 'n')
		{
			throw "Cannot save experiment";
		}
	}

	// if file doesn't exist (or user chooses to overwrite)
	ofstream datafile;
	datafile.open(filename);
	if (datafile.is_open())
	{
		std::map<std::string, experiment>::iterator ptr;
		// look for experiment with key n
		ptr = u.find(n);
		if (ptr != u.end())
		{
			// dereference the pointer
			experiment value = ptr->second;

			// iterating over the whole measurement container for the appropriate experiment
			for (auto vec_iter = value.measurementContainer.begin(); vec_iter != value.measurementContainer.end(); ++vec_iter)
			{
				// now iterate over the rows of type vector<measurement*>
				for (vector<measurement*>::iterator meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
				{
<<<<<<< HEAD
					// print
=======
					(*meas_it)->printInfo;
>>>>>>> 2fa91fc1dec28f0e2e53f02394ac546d128787c7
				}
			}
		}
		else
			throw "Could not find experiment";
	}
}

// function for a user to add an experiment by hand
void datans::addExperiment(std::map<std::string, experiment> u)
{
	std::string tempName, tempHeads, buf;
	std::vector<std::string> tempHeadings;

	// get experiment name
	std::cout << "Enter experiment name: ";
	std::cin >> tempName;
	std::cin.ignore();

	// get headings input and split on space delimiter
	std::cout << "\nEnter headings, space delimited: ";
	std::getline(std::cin, tempHeads);
	std::stringstream ss(tempHeads);
	while (ss >> buf)
	{
		tempHeadings.push_back(buf);
	}

	// create experiment with desired name and headings, then clean up
	experiment tempExp(tempName, tempHeadings);

	char flag('y');
	std::string tempdata;
	std::vector<string> tempMeasurement;
	std::vector<measurement*> rowMeasurement;
	static int counter{ 0 };

	do
	{
		for (unsigned int i = 0; i < tempHeadings.size(); ++i)
		{
			try {
				cout << "Enter data " << counter + 1 << " for " << tempHeadings[i] << " (space delimited):\n";
				std::getline(std::cin, tempdata);
				std::stringstream ss(tempdata);
				// create a vector to pass to the measurement object creator
				while (ss >> buf)
				{
					tempMeasurement.push_back(buf);
				}
				// call function to create measurement object from the tempMeasurement vector
				rowMeasurement.push_back(addMeasurement(tempMeasurement));
				tempMeasurement.clear();
			}
			catch (const char* msg) {
				cerr << msg << endl;
				std::cin.clear();
			}
			tempExp.measurementContainer.push_back(rowMeasurement);
			rowMeasurement.clear();
		}
		counter++;
		cout << "Continue to add data? (Current rows: " << counter << ") (Y/N): ";
		cin >> flag;
		cin.ignore();

	} while (flag == 'Y' || flag == 'y');

	tempHeadings.clear();
	u[tempName] = tempExp;
}

<<<<<<< HEAD
void datans::deleteExperiment(std::string n, std::map<std::string, experiment> u)
{

}

=======
>>>>>>> 2fa91fc1dec28f0e2e53f02394ac546d128787c7
void datans::readExperiment(std::string n, std::map<std::string, experiment> u)
{
	std::ifstream datafile;
	std::string templine, templine2, buf;
	std::vector<std::string> tempHeadings;
	std::vector<string> tempMeasurement;
	std::vector<measurement*> rowMeasurement;

	datafile.open(n);
	if (datafile.is_open())
	{
		// read the first line which will be headings
		std::getline(datafile, templine);
		std::stringstream ss(templine);
		while (ss >> buf)
		{
			tempHeadings.push_back(buf);
		}
		// create the temporary experiment from filename and headings (remove file extension)
		n.erase(n.end() - 4, n.end());
		experiment tempExp(n, tempHeadings);
		// position should now be on the second line
		while (std::getline(datafile, templine))
		{
			// we read the whole line, then each measurement is tab delineated, so split into n strings (each string is a measurement)
			// then create each measurement and push that back into a row. row is then pushed back into the measurement container
			std::stringstream ss(templine);
			while (getline(ss, templine2, '\t'))
			{
				// open a new stringstream containing just the info between tabs
				std::stringstream ss2(templine2);
				while (ss2 >> buf)
				{
					tempMeasurement.push_back(buf);
				}
				rowMeasurement.push_back(addMeasurement(tempMeasurement));
			}
			tempExp.measurementContainer.push_back(rowMeasurement);
		}
		tempHeadings.clear();
		u[n] = tempExp;
	}
	datafile.close();
}