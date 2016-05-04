#include "Header.h"

using namespace std;
using namespace datans;

// parse filenames of all files in directory (default ../data)
std::vector<std::string> readDir()
{
	std::vector<std::wstring> wnames;	// wide char version
	std::vector <std::string> names;
	wchar_t file_path[250];

	swprintf(file_path, 256, L"%s/*.txt", L".//data//");	// only want .txt files
	WIN32_FIND_DATA foundFile;
	HANDLE hFind = FindFirstFile(file_path, &foundFile);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			wnames.push_back(foundFile.cFileName);
		} while (FindNextFile(hFind, &foundFile));
		FindClose(hFind);
	}

	// converter to change wide chars to string
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	for (size_t i = 0; i < wnames.size(); ++i)
	{
		std::string str(wnames[i].begin(), wnames[i].end());
		names.push_back(str);
	}

	// if it looks like the directory is empty, try and create a new directory
	if (names.empty())
	{
		// default location ./data
		const wchar_t * path = L".//data//";
		if (!CreateDirectory(path, NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())		// will throw error if directory exists but is just empty or improper permissions
		{
			cerr << "Could not create data directory or directory is empty" << endl;
		}
		else
		{
			// program will create data directory
			cout << "Created directory data" << endl;
		}
	}

	return names;
}

// function to test whether file already exists
bool isFileExist(std::string &n)
{
	std::ifstream infile(".//data//" + n);
	return infile.good();
}

// print the list of experiments in memory
void datans::printExperimentList(std::map<std::string, Experiment> user)
{
	for (std::map<std::string, Experiment>::iterator it = user.begin(); it != user.end(); ++it)
	{
		std::cout << it->first << ", ";
	}
	cout << "\b\b): ";
}