#ifndef MY_HEADER
#define MY_HEADER

#include<iostream>
#include<stdlib.h> // for c style exit
#include<vector>
#include<string>
#include<sstream>
#include<ctime>
#include <ctype.h>
#include<fstream>
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
		virtual ~measurement(){};
		virtual void printInfo() = 0;	// print out that value
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
		numMeasure() : value(0), error(0), systError(0) { date = currentDate(); }
		numMeasure(double v, double e, double se, std::string d) : value(v), error(e), systError(se), date(d) {}
		~numMeasure() {}
		void printInfo();
	};

	class stringMeasure : public measurement
	{
	private:
		// string type measurements will not have any error attributed to them
		std::string value;
		std::string date;
	public:
		stringMeasure() : value("null") { date = currentDate();  }
		stringMeasure(std::string v, std::string d) : value(v), date(d) {}
		~stringMeasure() {}
		void printInfo();
	};

	class experiment
	{
	private:
		std::vector<std::string> headings;
		std::vector < std::vector<measurement*> > measurementContainer;
		std::string name;
	public:
		experiment() : name("null") { headings.push_back("null"); }
		experiment(std::string n, std::vector<std::string> v) : name(n) { for (auto iter = v.begin(); iter != v.end(); ++iter) { headings.push_back(*iter); } };
		~experiment() {}

		double errorCalc();
		
		// called by addExperiment function to parse a string and return
		// a pointer to a nummeasure or stringmeasure. can also be called
		// by readExperiment (to parse from a file)
		friend measurement* addMeasurement(std::vector<std::string> v);
		friend void printExperiment(std::string n, std::map<std::string, experiment> u);		// to print experiment to screen
		void saveExperiment(std::string n, std::map<std::string, experiment> u);				// to save experiments to file
		friend void addExperiment(std::map<std::string, experiment> u);							// to add experiments by hand
		friend void readExperiment(std::string n, std::map<std::string, experiment> &u);			// to read experiment from a file
		friend void deleteExperiment(std::string n, std::map<std::string, experiment> u);		// to delete an experiment
	};
}

#endif