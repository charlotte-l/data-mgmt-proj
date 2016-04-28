#ifndef MY_HEADER
#define MY_HEADER
#define NOMINMAX

#include<iostream>
#include <iomanip>
#include<stdlib.h> // for c style exit
#include<vector>
#include<string>
#include<sstream>
#include<ctime>
#include <ctype.h>
#include <iterator>
#include <memory>
#include<fstream>
#include <limits>
#include <stdio.h>
#include<map>
#include"dirent.h"

//prototype for directory reading function
std::vector<std::string> readDir();

// anonymous namespace to avoid collisions
namespace
{
	// function to return current date for default constructor
	const std::string currentDate()
	{
		time_t t = time(0);
		struct tm now;
		localtime_s(&now, &t);
		char buf[80];
		strftime(buf, sizeof(buf), "%Y-%m-%d", &now);
		return buf;
	}
}

namespace datans
{
	// all class definitions go here

	class measurement
	{
	public:
		virtual ~measurement(){ std::cout << "measurement desctructor called"; };
		virtual void printInfo() = 0;	// print out that value
		virtual double returnError() = 0;
		virtual double returnValue() = 0;
		virtual std::string saveInfo() = 0;
	};

	class numMeasure : public measurement
	{
	private:
		// numerical values will also have error and systematic errors
		double value;
		double error;
		double systError;
		std::string date;
	public:
		numMeasure() : value(0), error(0), systError(0) { date = currentDate(); std::cout << "numMeasure default constructor called\n"; }
		numMeasure(double v, double e, double se, std::string d) : value(v), error(e), systError(se), date(d) { std::cout << "numMeasure param constructor called\n"; }
		~numMeasure() { std::cout << "numMeasure destructor called\n"; }
		void printInfo() { std::cout << std::left << std::setw(5) << value << std::setw(5) << error << std::setw(5) << systError << std::setw(5) << date << std::setw(5); }
		std::string saveInfo() { std::string temp = std::to_string(value) + " "
			+ std::to_string(error) + " " + std::to_string(systError) + " " + date + "\t"; return temp; }
		double returnError() { return error + systError; }
		double returnValue() { return value; }
	};

	class stringMeasure : public measurement
	{
	private:
		// string type measurements will not have any error attributed to them
		std::string value;
		std::string date;
	public:
		stringMeasure() : value("null") { date = currentDate(); std::cout << "stringMeasure default constructor called\n"; }
		stringMeasure(std::string v, std::string d) : value(v), date(d) { std::cout << "stringMeasure param constructor called\n"; }
		~stringMeasure() { std::cout << "stringMeasure destructor called\n"; }
		void printInfo() { std::cout << std::left << std::setw(5) << value << std::setw(5) << date << std::setw(5); }
		std::string saveInfo() { std::string temp = value + " " + date + "\t"; return temp; }
		double returnError() { return 0; }
		double returnValue() { return 0; } // this function is only called in error calculation and so returns 0
	};

	class experiment
	{
	private:
		std::vector<std::string> headings;
		std::vector < std::vector<std::shared_ptr<measurement>> > measurementContainer;
		std::string name;
	public:
		experiment() : name("null") { headings.push_back("null"); std::cout << "experiment default constructor called\n"; }
		experiment(std::string n, std::vector<std::string> v) : name(n) { for (auto iter = v.begin(); iter != v.end(); ++iter) { headings.push_back(*iter); }
			std::cout << "experiment param constructor called\n"; };
		~experiment() { std::cout << "experiment object " << name << " destructor called\n"; }

		// move constructor - prototyped, function in Functions.cpp, used in addExperiment
		experiment(experiment&& e);
		// assignment operator - used in printExperiment


		std::vector<double> errorCalc();
		friend std::shared_ptr<measurement> addMeasurement(std::vector<std::string> v);
		friend void printExperiment(std::string n, std::map<std::string, experiment> u);				// to print experiment to screen
		void saveExperiment();																			// to save experiments to file
		friend void addExperiment(std::map<std::string, experiment> &u);								// to add experiments by hand
		friend void readExperiment(std::string n, std::map<std::string, experiment> &u, char flag);		// to read experiment from a file - flag determines filepath
		friend void deleteExperiment(std::string n, std::map<std::string, experiment> &u);				// to delete an experiment
	};
}

#endif