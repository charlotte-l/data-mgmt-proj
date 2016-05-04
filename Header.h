#ifndef MY_HEADER
#define MY_HEADER
#define NOMINMAX

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
#include<locale>
#include<codecvt>

const int WIDTH = 40;	// column with in experiment output
std::vector<std::string> readDir();
bool isFileExist(std::string &n);

namespace
{
	// prototyped here, definitions below class definitions
	std::string openFileDialogue();
	std::string fileNameFromPath(std::string &filePath);
	const std::string currentDate();
	void setConsoleSize(int w, int h);
}

namespace datans
{
	// class interface for storing individual measurements, either of
	// numerical or text base values
	class Measurement
	{
	public:
		virtual ~Measurement(){};
		virtual void printInfo(const char &seperator) = 0;
		virtual std::string saveInfo(char &flag) = 0;
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
		NumMeasure(double value, double error, double systError, std::string date) : value_(value), error_(error), systError_(systError), date_(date) {}
		~NumMeasure() {}
		// getters and setters
		void printInfo(const char &seperator);
		std::string saveInfo(char &flag);	// flag determines saving as .txt, .csv or .tex
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
		StringMeasure(std::string value, std::string date) : value_(value), date_(date){};
		~StringMeasure(){}
		// getters and setters
		void printInfo(const char &seperator);
		std::string saveInfo(char &flag);	// flag determines saving as .txt, .csv or .tex
		double getError() { return 0; }
		double getValue() { return 0; } // this function is only called in error calculation and so returns 0
		int updateInfo(std::vector<std::string> &v);
	};

	// class to store measurements in a container and manipulate
	class Experiment
	{
	private:
		std::vector<std::string> headings_;
		std::vector<std::string> dataHeadings_;		// if data is num or string, it will have a header of that type
		std::vector < std::vector<std::shared_ptr<Measurement>> > measurementContainer_;
		std::string name_;
	public:
		Experiment() : name_("null") { headings_.push_back("null"); }
		Experiment(std::string name, std::vector<std::string> v);
		~Experiment() {}
		Experiment(Experiment&& e);

		std::vector<double> errorCalc();
		friend std::shared_ptr<Measurement> addMeasurement(std::vector<std::string> v);
		friend void readExperiment(std::string n, std::map<std::string, Experiment> &u, char readFlag);		// flag determines filepath
		friend void addExperiment(std::map<std::string, Experiment> &u);
		int printExperiment();
		int editExperiment();
		int saveExperiment(char flag);
		int deleteExperiment(std::map<std::string, Experiment> &u);
	};

	void printExperimentList(std::map<std::string, Experiment> user);
}

namespace
{
	// returns current date
	const std::string currentDate()
	{
		time_t t = time(0);
		struct tm now;
		localtime_s(&now, &t);
		char buf[80];
		strftime(buf, sizeof(buf), "%Y-%m-%d", &now);
		return buf;
	}

	// creates a win32api dialogue to return filepath for opening files
	std::string openFileDialogue()
	{
		OPENFILENAME ofn;
		HWND whnd = GetConsoleWindow();
		wchar_t buf[250];

		// initialise OPENFILENAME struct vals
		// use literals as program is not Unicode by default

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = whnd;
		ofn.lpstrFile = buf;
		ofn.nMaxFile = sizeof(buf);
		ofn.lpstrFilter = L"Text\0*.txt\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.lpstrDefExt = L"txt";
		ofn.lpstrFile[0] = '\0';

		if (!GetOpenFileNameW(&ofn))
		{
			return "";
		}

		// converter to change wide chars to string
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;
		std::string filePath = converter.to_bytes(buf);
		return filePath;
	}

	// parse filepath from openFileDialogue into a name only
	std::string fileNameFromPath(std::string &filePath)
	{
		const int position = filePath.find_last_of("\\/");
		if (std::string::npos != position)
		{
			std::string fileName(filePath.substr(position + 1));
			fileName.erase(fileName.end() - 4, fileName.end());
			return fileName;
		}
		else
		{
			return "";
		}
	}

	// win32api handle getter-setter to resize console window
	void setConsoleSize(int w, int h)
	{
		HWND wh = GetConsoleWindow();
		MoveWindow(wh, 100, 100, w, h, TRUE);
	}
}
#endif