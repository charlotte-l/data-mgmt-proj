#include "Header.h"
using namespace std;
using namespace datans;

void NumMeasure::printInfo(const int &width, const char &seperator)
{
	std::cout << std::left << std::setw((width/5)-1) << std::setfill(seperator) << value_;
	std::cout << std::left << std::setw((width / 5) - 1) << std::setfill(seperator) << error_;
	std::cout << std::left << std::setw((width / 5) - 1) << std::setfill(seperator) << systError_;
	std::cout << std::left << std::setw((width / 5) + 4) << std::setfill(seperator) << date_;
	std::cout << std::left << std::setw((width / 5) - 1) << std::setfill(seperator) << "";
}

std::string NumMeasure::saveInfo(char &flag)
{
	std::string temp;
	switch (flag)
	{
	case 't':
		temp = std::to_string(value_) + " " + std::to_string(error_)
			+ " " + std::to_string(systError_) + " " + date_ + "\t";
		return temp;

	case 'c':
		temp = std::to_string(value_) + "," + std::to_string(error_)
			+ "," + std::to_string(systError_) + "," + date_ + "," + " ";
		return temp;

	case 'l':
		// figure out latex formatting here
		temp = " ";
		return temp;
	}
}

int NumMeasure::updateInfo(std::vector<std::string> &v)
{
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
		cout << "Cannot convert numerical measurement to textual measurement\n" << endl;
		return -1;
	}
}

void StringMeasure::printInfo(const int &width, const char &seperator)
{
	std::cout << std::left << std::setw(width / 2) << std::setfill(seperator) << value_;
	std::cout << std::left << std::setw((width/2) - 5) << std::setfill(seperator) << date_;
	std::cout << std::left << std::setw((width/2) - 15) << std::setfill(seperator) << "";
}

std::string StringMeasure::saveInfo(char &flag)
{
	std::string temp = value_ + " " + date_ + "\t";
	return temp;
}

int StringMeasure::updateInfo(std::vector<std::string> &v)
{
	if (v.size() == 1)
	{
		value_ = v[0];
		date_ = currentDate();
		return 1;
	}
	else
	{
		cout << "Cannot convert textual measurement to numerical measurement\n" << endl;
		return -1;
	}
}