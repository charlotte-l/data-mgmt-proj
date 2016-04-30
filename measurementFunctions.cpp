#include "Header.h"
using namespace std;
using namespace datans;

void NumMeasure::printInfo()
{
	std::cout << std::left << std::setw(5) << value_ << std::setw(5) << error_
		<< std::setw(5) << systError_ << std::setw(5) << date_ << std::setw(5);
}

std::string NumMeasure::saveInfo()
{
	std::string temp = std::to_string(value_) + " " + std::to_string(error_)
		+ " " + std::to_string(systError_) + " " + date_ + "\t";
	return temp;
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

void StringMeasure::printInfo()
{
	std::cout << std::left << std::setw(5) << value_ << std::setw(5) << date_ << std::setw(5);
}

std::string StringMeasure::saveInfo()
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
