#ifndef MY_HEADER
#define MY_HEADER
#define NOMINMAX
#define WIDTH 40;

#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<sstream>
#include<fstream>
#include<ctime>
#include<iterator>
#include<memory>
#include<map>
#include<algorithm>
#include"dirent.h"
#include <windows.h>

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

	// win32api handles to resize console window
	void setConsoleSize(int w, int h)
	{
		HWND wh = GetConsoleWindow();			// get window handle
		MoveWindow(wh, 100, 100, w, h, TRUE);	// resize
	}
}

namespace datans
{
	class Measurement
	{
	public:
		virtual ~Measurement(){};
		virtual void printInfo(const int &width, const char &seperator) = 0;
		virtual std::string saveInfo() = 0;
		virtual double getError() = 0;
		virtual double getValue() = 0;
		virtual int updateInfo(std::vector<std::string> &v) = 0;
	};

	class NumMeasure : public Measurement
	{
	private:
		double value_;
		double error_;
		double systError_;
		std::string date_;
	public:
		NumMeasure() : value_(0), error_(0), systError_(0) { date_ = currentDate(); }
		NumMeasure(double value, double error, double systError, std::string date) : value_(value), error_(error), systError_(systError), date_(date) {}
		~NumMeasure() {}
		void printInfo(const int &width, const char &seperator);
		std::string saveInfo();
		double getError() { return error_ + systError_; }
		double getValue() { return value_; }
		int updateInfo(std::vector<std::string> &v);
	};

	class StringMeasure : public Measurement
	{
	private:
		std::string value_;
		std::string date_;
		char dateflag_;
	public:
		StringMeasure() : value_("null") { date_ = currentDate(); }
		StringMeasure(std::string value, std::string date) : value_(value), date_(date){};
		~StringMeasure(){}
		void printInfo(const int &width, const char &seperator);
		std::string saveInfo();
		double getError() { return 0; }
		double getValue() { return 0; } // this function is only called in error calculation and so returns 0
		int updateInfo(std::vector<std::string> &v);
	};

	class Experiment
	{
	private:
		std::vector<std::string> headings_;
		std::vector<std::string> dataHeadings_;
		std::vector < std::vector<std::shared_ptr<Measurement>> > measurementContainer_;
		std::string name_;
	public:
		Experiment() : name_("null") { headings_.push_back("null"); }
		Experiment(std::string name, std::vector<std::string> v);
		~Experiment() {}

		// move constructor
		Experiment(Experiment&& e);

		std::vector<double> errorCalc();
		friend std::shared_ptr<Measurement> addMeasurement(std::vector<std::string> v);
		friend int readExperiment(std::string &n, std::map<std::string, Experiment> &u, char readFlag);		// flag determines filepath
		friend int addExperiment(std::map<std::string, Experiment> &u);
		friend int printExperiment(std::string &n, std::map<std::string, Experiment> u);
		friend int editExperiment(std::string &n, std::map<std::string, Experiment> u);
		int saveExperiment();
		friend int deleteExperiment(std::string &n, std::map<std::string, Experiment> &u);
	};
}

#endif