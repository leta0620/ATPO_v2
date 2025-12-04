#include "OuterInput.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

bool OuterInput::ParsePatternFile() {
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

		if (spaceCount == 0)
		{
			stringstream ss(line);
			string tmp;
			while(ss >> tmp)
			{
				if (tmp.rbegin()[0] == ',')
					tmp = tmp.substr(0, tmp.size() - 1);
				this->initialPatternTable.push_back(tmp);
			}
		}
		else if (spaceCount == 1)
		{
			stringstream ss(line);
			string tmp;
			while (ss >> tmp)
			{
				if (tmp.rbegin()[0] == ',')
					tmp = tmp.substr(0, tmp.size() - 1);
				this->initialPatternRotationTable.push_back(tmp);
			}
		}
		else if (spaceCount == 2)
		{
			stringstream ss(line);
			string instName, labelName;
			ss >> instName >> labelName;
			instName = instName.substr(0, instName.size() - 1);

			this->labelNameMapInstName[labelName] = instName;
			this->instNameMapLabelName[instName] = labelName;
		}
	}

	patternStream.close();
	return true;
}

bool OuterInput::ParseCdlFile() {
	ifstream cdlStream(cdlFile);
	if (!cdlStream.is_open()) {
		cerr << "Error: Unable to open CDL file: " << cdlFile << endl;
		return false;
	}

	// Implementation for parsing the CDL file
	string line;
	while (cin >> line)
	{
		stringstream ss(line);

		if (line == "")
		{
			continue;
		}

		string tmp;
		ss >> tmp;
		if (tmp == ".SUBCKT")
		{
			ss >> tmp;
			string subcktName = tmp;
						
			if (ss.eof())
			{
				// target subskt
			}
			else
			{
				// set instNameMapCdlPosition
				string d, g, s, b, m;
				ss >> d >> g >> s >> b >> m;
				m = m.substr(2, m.size() - 3);
				
				this->instNameMapCdlPosition[subcktName] = make_tuple(d, s, g, b, stoi(m));
			}
		}

	}

	cdlStream.close();

	return true;
}