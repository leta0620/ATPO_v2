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
		if (line.empty())
		{
			spaceCount++;
			continue;
		}

		if (spaceCount == 0)
		{
			vector<string> newLine;
			stringstream ss(line);
			string tmp;
			while(ss >> tmp)
			{
				if (tmp.rbegin()[0] == ',')
					tmp = tmp.substr(0, tmp.size() - 1);
				newLine.push_back(tmp);
			}

			this->initialPatternTable.push_back(newLine);
		}
		else if (spaceCount == 1)
		{
			vector<string> newLine;
			stringstream ss(line);
			string tmp;
			while (ss >> tmp)
			{
				if (tmp.rbegin()[0] == ',')
					tmp = tmp.substr(0, tmp.size() - 1);
				newLine.push_back(tmp);
			}

			this->initialPatternRotationTable.push_back(newLine);
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
	while (getline(cdlStream, line))
	{
		stringstream ss(line);
		if (!line.empty() && line[0] == ' ')
		{
			line = line.substr(1, line.size() - 1);
		}

		if (line.empty() || line[0] == '*')
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
				while (getline(cdlStream, line))
				{
					stringstream ssT(line);
					string instName, pin1, pin2, pin3, pin4, skip, cellType, m;

					if (line == ".ENDS" || line == ".ends") break;

					if (ssT >> instName >> pin1 >> pin2 >> pin3 >> pin4 >> skip >> cellType >> m)
					{
						// 移除實例名稱前的 'X'
						if (!instName.empty() && instName[0] == 'X')
							instName = instName.substr(1);

						// 移除 m= 前綴並處理可能的尾巴
						if (m.size() > 2 && m.rfind("m=", 0) == 0)
						{
							m = m.substr(2); // 從 index 2 開始到結尾
							// 若有結尾的 ')' 或其他，嘗試移除非數字尾字元
							while (!m.empty() && !isdigit(static_cast<unsigned char>(m.back())))
								m.pop_back();
						}

						// 檢查是否有該 instName 的 cell-type 定義
						auto it = this->instNameMapCellInformation.find(cellType);
						if (it != this->instNameMapCellInformation.end())
						{
							// 取得已登記的 cell type pin 訊息 (tuple<string,string,string,string,int>)
							auto &cellTypeInformation = it->second;
							string cT1, cT2, cT3, cT4;
							int cTM;
							tie(cT1, cT2, cT3, cT4, cTM) = cellTypeInformation;

							string dNet, gNet, sNet, bNet;
							// 正確實作的 lambda：以 cTPin 決定要把哪個 net 指派給哪個變數
							auto SetPinNets = [&dNet, &gNet, &sNet, &bNet](const string &cTPin, const string &pin)
							{
								if (cTPin == "D")
								{
									dNet = pin;
								}
								else if (cTPin == "G")
								{
									gNet = pin;
								}
								else if (cTPin == "S")
								{
									sNet = pin;
								}
								else if (cTPin == "B")
								{
									bNet = pin;
								}
							};

							// 依照 cell type 的定義把對應的 pin 指派到 d/g/s/b
							SetPinNets(cT1, pin1);
							SetPinNets(cT2, pin2);
							SetPinNets(cT3, pin3);
							SetPinNets(cT4, pin4);

							this->instStructList.push_back({ dNet, gNet, sNet, bNet, stoi(m) * cTM, instName, this->instNameMapLabelName[instName] });
						}
						else
						{
							// default celltype pin information
							string dNet = pin1, gNet = pin2, sNet = pin3, bNet = pin4;

							this->instStructList.push_back({ dNet, gNet, sNet, bNet, stoi(m), instName, this->instNameMapLabelName[instName] });
						}
					}
					else
					{
						cerr << "Error: Invalid subckt instance line: " << line << endl;
						return false;
					}
				}
			}
			else
			{
				// set instNameMapCdlPosition
				string pin1, pin2, pin3 ,pin4, m;
				ss >> pin1 >> pin2 >> pin3 >> pin4 >> m;

				m = m.substr(2, m.size() - 3);
				
				this->instNameMapCellInformation[subcktName] = make_tuple(pin1, pin2, pin3, pin4, stoi(m));
			}

			while (getline(cdlStream, line))
			{
				if (line == ".ENDS" || line == ".ends") break;
				else continue;
			}
		}

	}

	cdlStream.close();

	return true;
}

bool OuterInput::GenIntermidiateFile() {
	ofstream outfile(intermidiateFile);

	if (!outfile.is_open()) {
		cerr << "Error: Unable to open intermidiate file: " << intermidiateFile << endl;
		return false;
	}

	// Write instStructList to intermidiate file

	outfile.close();
	return true;
}