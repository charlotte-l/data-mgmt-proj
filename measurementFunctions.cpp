#include "Header.h"
using namespace std;
using namespace datans;

void NumMeasure::printInfo(const char &seperator)
{
	// sets left align, column width and seperation between to enforce column width
	std::cout << std::left << std::setw((WIDTH / 5) - 1) << std::setfill(seperator) << std::setprecision(5) << value_;
	std::cout << std::left << std::setw((WIDTH / 5) - 1) << std::setfill(seperator) << std::setprecision(5) << error_;
	std::cout << std::left << std::setw((WIDTH / 5) - 1) << std::setfill(seperator) << std::setprecision(5) << systError_;
	std::cout << std::left << std::setw((WIDTH / 5) + 4) << std::setfill(seperator) << date_;
	std::cout << std::left << std::setw((WIDTH / 5) - 1) << std::setfill(seperator) << "";
}

std::string NumMeasure::saveInfo(char &flag)
{
	std::string temp;
	// change formatting based on if saving to .txt, .csv or .tex
	switch (flag)
	{
		// .txt file
		case 't':
		{
			temp = std::to_string(value_) + " " + std::to_string(error_)
				+ " " + std::to_string(systError_) + " " + date_ + "\t";
			return temp;
		}
		//.csv file
		case 'c':
		{
			temp = std::to_string(value_) + "," + std::to_string(error_)
				+ "," + std::to_string(systError_) + "," + date_ + "," + " ";
			return temp;
		}
		// .tex file
		case 'l':
		{
			// stringstream to set # decimal places, \\tab is Latex command
			std::stringstream str;
			str << fixed << setprecision(3) << value_ << "\\tab " << error_ << "\\tab " << systError_ << "\\tab " << date_;
			temp = str.str();
			return temp;
		}

		default:
		{
			perror("Invalid input");
			return " ";
		}
	}
}

int NumMeasure::updateInfo(std::vector<std::string> &v)
{
	// numerical type measurement
	if (v.size() == 3)
	{
		value_ = stod(v[0]);
		error_ = stod(v[1]);
		systError_ = stod(v[2]);
		date_ = currentDate();
		return 1;
	}
	else
	{
		// should only be called on a nummeasure type, so error otherwise
		cout << "Cannot convert numerical measurement to textual measurement\n" << endl;
		return -1;
	}
}

void StringMeasure::printInfo(const char &seperator)
{
	// sets left align, column width and seperation between to enforce column width
	std::cout << std::left << std::setw(WIDTH / 2) << std::setfill(seperator) << value_;
	std::cout << std::left << std::setw((WIDTH / 2) - 5) << std::setfill(seperator) << date_;
	std::cout << std::left << std::setw((WIDTH / 2) - 15) << std::setfill(seperator) << "";
}

std::string StringMeasure::saveInfo(char &flag)
{
	std::string temp;
	// change formatting based on if saving to .txt, .csv or .tex
	switch (flag)
	{
		// .txt file
		case 't':
		{
			temp = value_ + " " + date_ + "\t";
			return temp;
		}
		// .csv file
		case 'c':
		{
			temp = value_ + "," + date_ + "," + " ";
			return temp;
		}
		// .tex file
		case 'l':
		{
			// \\tab is Latex command
			temp = value_ + "\\tab " + date_;
			return temp;
		}

		default:
		{
			perror("Invalid input");
			return " ";
		}
	}
}

int StringMeasure::updateInfo(std::vector<std::string> &v)
{
	// string type measurement
	if (v.size() == 1)
	{
		value_ = v[0];
		date_ = currentDate();
		return 1;
	}
	else
	// should only be called on a stringmeasure type, so error otherwise
	{
		cout << "Cannot convert textual measurement to numerical measurement\n" << endl;
		return -1;
	}
}