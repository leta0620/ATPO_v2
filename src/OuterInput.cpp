#include "OuterInput.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

bool OuterInput::ParsePatternFile() {
	// Implementation for parsing the pattern file
	// This is a placeholder implementation
	ifstream patternStream(patternFile);

	if (!patternStream.is_open()) {
		cerr << "Error: Unable to open pattern file: " << patternFile << endl;
		return false;
	}

	string line;
	int spaceCount = 0;
	while (getline(patternStream, line))
	{
		if (line == "")
		{
			spaceCount++;
			continue;
		}

		switch (spaceCount)
		{
		case 0:
			stringstream ss(line);
			string tmp;
			while(ss >> tmp)
			{
				if (tmp.rbegin()[0] == ',')
					tmp = tmp.substr(0, tmp.size() - 1);
				this->initialPatternTable.push_back(tmp);
			}

			break;
		case 1:
			stringstream ss(line);
			string tmp;
			while (ss >> tmp)
			{
				if (tmp.rbegin()[0] == ',')
					tmp = tmp.substr(0, tmp.size() - 1);
				this->initialPatternRotationTable.push_back(tmp);
			}

			break;
		default:
			break;
		}
	}



	patternStream.close();
	return true;
}

bool OuterInput::ParseCdlFile() {
	// Implementation for parsing the CDL file
	// This is a placeholder implementation
	return true;
}