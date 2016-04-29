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
			// ignore '.' and '..' directory references
			if ((*ent->d_name) != '.' && (*ent->d_name) != '..')
			{
				filelist.push_back(ent->d_name);
			}
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

// function to test whether file already exists
bool is_file_exist(std::string &n)
{
	std::ifstream infile(".//data//" + n + ".txt");
	return infile.good();
}


// rvalue move constructor for efficient moving
experiment::experiment(experiment&& e)
{
	headings = e.headings;
	measurementContainer = std::move(e.measurementContainer);
	dataHeadings = e.dataHeadings;
	name = e.name;

	e.headings.clear();
	e.dataHeadings.clear();
	e.measurementContainer.clear();
	e.name = "";
}

vector<double> experiment::errorCalc()
{
	// this will be called by printexperiment function

	double rowcount{ 0 };
	int index{ 0 };

	// check container size
	int colcount = headings.size();

	// then make a vector that big
	vector<double> errors(colcount);

	// iterate over each row in measurementContainer
	for (auto vec_iter = measurementContainer.begin(); vec_iter != measurementContainer.end(); ++vec_iter)
	{
		rowcount++;
		// now iterate over the measurements within each rows of type vector<measurement>
		for (auto meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
		{
			// we need to sum error / value for the COLUMN - so for each element of the row
			// we will add to the vector error in the same row format
			index = std::distance((*vec_iter).begin(), meas_it);
			if ((*meas_it)->returnValue() != 0)
			{
				errors[index] += ((*meas_it)->returnError() / (*meas_it)->returnValue());
			}
		}
	}

	for (auto iter = errors.begin(); iter != errors.end(); ++iter)
	{
		(*iter) = (*iter) / rowcount;
	}
	return errors;
}

std::shared_ptr<measurement> datans::addMeasurement(std::vector<std::string> v)
{
	int vectorSize = v.size();
	if (vectorSize == 4)
	{
		// convert to relevant type
		double tempval = stod(v[0]);
		double temperr = stod(v[1]);
		double tempsyst = stod(v[2]);
		// create the measurement
		std::shared_ptr<measurement> ptr (new numMeasure(tempval, temperr, tempsyst, v[3]));
		return ptr;
	}
	else if (vectorSize == 2)
	{
		std::shared_ptr<measurement> ptr (new stringMeasure(v[0], v[1]));
		return ptr;
	}
	else
	{
		throw "Invalid data type! Format should be (numerical value, error, systematic error, date) or (textual value, date)\n";
	}
}

void datans::printExperiment(std::string n, std::map<std::string, experiment> u)
{
	std::map<std::string, experiment>::iterator ptr;
	// look for experiment with key n
	ptr = u.find(n);
	
	if (ptr != u.end())
	{
		// print out the headings first
		for (auto vec_iter = (ptr->second).headings.begin(); vec_iter != (ptr->second).headings.end(); ++vec_iter)
		{
			cout << std::left << ::setw(20) << (*vec_iter);
		}
		cout << endl;

		// now print out under each heading a column title based on the first element
		for (auto vec_iter = (ptr->second).dataHeadings.begin(); vec_iter != (ptr->second).dataHeadings.end(); ++vec_iter)
		{
			cout << std::left << std::setw(20) << (*vec_iter) << std::setw(10) << "\t";
		}
		cout << endl;

		// iterate over each row in measurementContainer
		for (auto vec_iter = (ptr->second).measurementContainer.begin(); vec_iter != (ptr->second).measurementContainer.end(); ++vec_iter)
		{
			// now iterate over the measurements within each rows of type vector<measurement*>
			for (vector<std::shared_ptr<measurement>>::iterator meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
			{
				// print info
				(*meas_it)->printInfo();
				std::cout << "\t";
			}
			cout << endl;
		}
		
		// finally print out % error
		std::vector<double> errors = (ptr->second).errorCalc();
		for (auto iter = errors.begin(); iter != errors.end(); ++iter)
		{
			if ((*iter) != 0)
			{
				cout << std::left << setw(5) << "% error: " << setw(5);
				cout << (*iter) << "\t";
			}
			else
				cout << std::left << setw(5) << "% error: N/A" << setw(5) << "\t";
		}
		cout << endl;
	}
	else
		throw "Experiment not found";
}

// function to save experiments to a file
void experiment::saveExperiment()
{
	std::string filename = name + ".txt";		// append .txt to the experiment name
	
	// check if file exists; if so, check if user wants to overwrite or not
	if (is_file_exist(name) == true)
	{
		char flag;
		cout << "File " << filename << " already exists. Overwrite? (Y/N): ";
		cin >> flag;

		if (flag == 'N' || flag == 'n')
		{
			throw "Cannot save experiment";
		}
	}

	// if file doesn't exist (or user chooses to overwrite)
	ofstream datafile;
	datafile.open(".//data//" + filename);
	if (datafile.is_open())
	{
		// write headings to datafile
		for (auto vec_iter = headings.begin(); vec_iter != headings.end(); ++vec_iter)
		{
			datafile << (*vec_iter) << "\t";
		}
		datafile << endl;

		// now the actual measurements
		// iterating over the whole measurement container for the appropriate experiment
		
		for (auto& vec_iter = measurementContainer.begin(); vec_iter != measurementContainer.end(); ++vec_iter)
		{
			// now iterate over the rows of type vector<measurement*>
			for (auto& meas_it = (*vec_iter).begin(); meas_it != (*vec_iter).end(); ++meas_it)
			{
				// write info to file
				datafile << (*meas_it)->saveInfo();
			}
			datafile << endl;
		}
		datafile.close();
	}
	else
		throw "Could not find experiment";
}

// function for a user to add an experiment by hand
void datans::addExperiment(std::map<std::string, experiment> &u)
{
	std::string tempName, tempHeads, buf;
	std::vector<std::string> tempHeadings;

	// get experiment name
	std::cout << "Enter experiment name: ";
	std::getline(std::cin, tempName);

	// get headings input and split on space delimiter
	std::cout << "\nEnter headings, space delimited - units in () i.e. Voltage(V): ";
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
	
	std::vector<std::shared_ptr<measurement>> rowMeasurement;
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
				if (tempMeasurement.size() == 4)
				{
					tempExp.dataHeadings[i] = "Value  Err  Systerr  Date";
				}
				else
				{
					tempExp.dataHeadings[i] = "Value  Date";
				}
				// call function to create measurement object from the tempMeasurement vector
				rowMeasurement.push_back(std::move(addMeasurement(tempMeasurement)));
				tempMeasurement.clear();
			}
			catch (const char* msg) {
				cerr << msg << endl;
				std::cin.clear();
			}
			tempExp.measurementContainer.emplace_back(std::move(rowMeasurement));
			rowMeasurement.clear();
		}
		counter++;
		cout << "Continue to add data? (Current rows: " << counter << ") (Y/N): ";
		cin >> flag;
		cin.ignore();
		
	} while (flag == 'Y' || flag == 'y');
	
	try {
		tempExp.saveExperiment();
	}
	catch (const char* msg) {
		cerr << msg << endl;
	}
	u.insert(std::make_pair(tempName, std::move(tempExp)));
	
}

void datans::deleteExperiment(std::string n, std::map<std::string, experiment> &u)
{
	char deleteFlag;
	cout << "Confirm deletion of experiment " << n << " (Y/N): " << endl;
	cin >> deleteFlag;
	if (tolower(deleteFlag) == 'y')
	{
		// create a string to filepath then pass this as a const char * to remove
		std::string filePath = ".//data//" + n + ".txt";
		if (remove(filePath.c_str()) != 0)
			// print error file not found
			perror("Error deleting data file for experiment");
		else
		{
			// delete the pair from the map
			u.erase(n);
			cout << "Experiment deleted successfully" << endl;
		}
	}
	else
	{
		cout << "Aborting deletion" << endl;
		return;
	}
}

int datans::readExperiment(std::string n, std::map<std::string, experiment> &u, char flag)
{
	std::ifstream datafile;
	std::string templine, templine2, buf, filePath;
	std::vector<std::string> tempHeadings;
	std::vector<std::string> tempDataHeadings;
	std::vector<string> tempMeasurement;
	std::vector<shared_ptr<measurement>> rowMeasurement;

	if (flag == 'r')
	{
		datafile.open(".//data//" + n);
	}

	else if (flag == 'f')
	{
		cout << "Enter filepath to file location (i.e. C://datafiles// - N.B double slash!): ";
		cin.ignore();
		getline(cin,filePath);
		datafile.open(filePath + n);
	}

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
		tempDataHeadings.resize(tempHeadings.size());
		// position should now be on the second line
		while (std::getline(datafile, templine))
		{
			int colno{ 0 };
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
				if (tempMeasurement.size() == 4)
				{
					tempDataHeadings[colno] = "Val  Err  SEerr  Date";
				}
				else
				{
					tempDataHeadings[colno] = "Val  Date";
				}
				colno++;
				rowMeasurement.push_back(std::move(addMeasurement(tempMeasurement)));
				tempMeasurement.clear();
			}
			tempExp.measurementContainer.emplace_back(std::move(rowMeasurement));
		}
		tempHeadings.clear();
		
		// if flag is f, save the experiment locally
		if (flag == 'f')
		{
			tempExp.saveExperiment();
		}
		
		u.insert(std::make_pair(n, std::move(tempExp)));
		u[n].dataHeadings = tempDataHeadings;
		datafile.close();
		return 1;
	}
	else
	{
		cout << "Could not find or read file " << n << endl;
		return -1;
	}

}