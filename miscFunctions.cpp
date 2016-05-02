#include "Header.h"

using namespace std;
using namespace datans;

std::vector<std::string> readDir()
{
	DIR *dir;
	struct dirent *ent;
	vector<std::string> fileList;
	if ((dir = opendir("./data")))
	{
		// print all files and directories in data
		while (ent = readdir(dir))
		{
			// ignore '.' and '..' directory references
			if ((*ent->d_name) != '.' && (*ent->d_name) != '..')
			{
				fileList.push_back(ent->d_name);
			}
		}
		closedir(dir);
		return fileList;
	}
	else
	{
		/* could not open directory */
		perror("Could not find data directory. Creating new directory...");
		return fileList;
	}
}

// function to test whether file already exists
bool isFileExist(std::string &n)
{
	std::ifstream infile(".//data//" + n + ".txt");
	return infile.good();
}

void datans::printExperimentList(std::map<std::string, Experiment> user)
{
	for (std::map<std::string, Experiment>::iterator it = user.begin(); it != user.end(); ++it)
	{
		std::cout << it->first << ", ";
	}
	cout << "\b\b): ";
}