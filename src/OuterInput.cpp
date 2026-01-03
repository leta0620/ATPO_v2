#include "OuterInput.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

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
		if (tmp == ".SUBCKT" || tmp == ".subskt")
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
						else if (m.size() > 2 && m.rfind("simM=", 0) == 0)
						{
							m = m.substr(5); // 從 index 5 開始到結尾
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
							int cTM = 0, cascodeInterInst = 0;
							tie(cT1, cT2, cT3, cT4, cTM, cascodeInterInst) = cellTypeInformation;

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

							if (this->hasLable)
							{
								this->instStructList.push_back({ dNet, gNet, sNet, bNet, stoi(m) * cTM, instName, this->instNameMapLabelName[instName], cellType, cascodeInterInst});
							}
							else
							{
								this->instStructList.push_back({ dNet, gNet, sNet, bNet, stoi(m) * cTM, instName, "", cellType, cascodeInterInst});
							}

							
						}
						else
						{
							// default celltype pin information
							string dNet = pin1, gNet = pin2, sNet = pin3, bNet = pin4;

							if (this->hasLable)
							{
								this->instStructList.push_back({ dNet, gNet, sNet, bNet, stoi(m), instName, this->instNameMapLabelName[instName], cellType, 1 });
							}
							else
							{
								this->instStructList.push_back({ dNet, gNet, sNet, bNet, stoi(m), instName, "", cellType, 1 });
							}
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
				ss >> pin1 >> pin2 >> pin3 >> pin4;
				if (!ss.eof()) ss >> m;
				else {
					m = "m=1";
				}

				//m = m.substr(2, m.size() - 3);
				if (m.size() > 2 && m.rfind("m=", 0) == 0)
				{
					m = m.substr(2); // 從 index 2 開始到結尾
					// 若有結尾的 ')' 或其他，嘗試移除非數字尾字元
					while (!m.empty() && !isdigit(static_cast<unsigned char>(m.back())))
						m.pop_back();
				}

				int cascodeInterInst = 0;
				while (getline(cdlStream, line))
				{
					if (line == ".ENDS" || line == ".ends") break;

					if (!line.empty())
					{
						// 確認最後兩個字是不是r2或R2
						if (line.size() >= 2)
						{
							string lastTwoChars = line.substr(line.size() - 2);
							if (/*lastTwoChars == "r2" || */lastTwoChars == "R2")
							{
								cascodeInterInst += 2;
								continue;
							}
						}
						cascodeInterInst++;
					}
				}
				
				this->instNameMapCellInformation[subcktName] = make_tuple(pin1, pin2, pin3, pin4, stoi(m), cascodeInterInst);
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

	outfile << "DEVICE_BEGIN\n";

	unordered_map<string, string> instNameMapSelfLable;
	char selfLabelChar = 'A';
	for (const auto& inst : this->instStructList)
	{
		if (hasLable)
		{
			outfile << inst.instName << " " << inst.labelName << " " << inst.cellType << " " << inst.m << " " << inst.cascodeInterInst << "\n";
		}
		else 
		{
			instNameMapSelfLable[inst.instName] = string(1, selfLabelChar);
			selfLabelChar++;
			outfile << inst.instName << " " << instNameMapSelfLable[inst.instName] << " " << inst.cellType << " " << inst.m << " " << inst.cascodeInterInst <<"\n";
		}

	}
	outfile << "END_DEVICE\n";
	
	outfile << "NET_BEGIN\n";


	map<string, vector<string>> netNameMapLabelDotPinDS;
	map<string, vector<string>> netNameMapLabelDotPinG;

	if (this->hasLable)
	{
		for (const auto& inst : this->instStructList)
		{
			netNameMapLabelDotPinDS[inst.dNet].push_back(inst.labelName + ".D");
			netNameMapLabelDotPinG[inst.gNet].push_back(inst.labelName + ".G");
			netNameMapLabelDotPinDS[inst.sNet].push_back(inst.labelName + ".S");
		}
	}
	else
	{
		for (const auto& inst : this->instStructList)
		{
			netNameMapLabelDotPinDS[inst.dNet].push_back(instNameMapSelfLable[inst.instName] + ".D");
			netNameMapLabelDotPinG[inst.gNet].push_back(instNameMapSelfLable[inst.instName] + ".G");
			netNameMapLabelDotPinDS[inst.sNet].push_back(instNameMapSelfLable[inst.instName] + ".S");
		}
	}

	// find common source
	vector<string>* commonSourceNets = nullptr;
	outfile << "COMMON_NODE_BEGIN\n";
	int mostCommonSourceCount = -1;
	for (auto& dsNetConnection : netNameMapLabelDotPinDS)
	{
		auto& netName = dsNetConnection.first;
		auto& labelDotPinList = dsNetConnection.second;
		if (int(labelDotPinList.size()) > mostCommonSourceCount)
		{
			bool skip = false;
			for (const auto& labelDotPin : labelDotPinList)
			{
				if (labelDotPin.back() != 'S')
				{
					skip = true;
					break;
				}
			}
			if (skip) continue;

			mostCommonSourceCount = labelDotPinList.size();
			commonSourceNets = &labelDotPinList;
		}
	}

	if (commonSourceNets == nullptr)
	{
		cerr << "Error: No common source net found." << endl;
		outfile.close();
		return false;
	}
	else 
	{
		outfile << ".net_S ";
		for (const auto& labelDotPin : *commonSourceNets)
		{
			outfile << labelDotPin << " ";

			if (labelDotPin.back() != 'S')
			{
				cerr << "Error: Common source net contains non-source pin: " << labelDotPin << endl;
				outfile.close();
				return false;
			}
		}
		outfile << "\n";

		for (const auto& dsConnection : netNameMapLabelDotPinDS)
		{
			const auto& netName = dsConnection.first;
			const auto& labelDotPinList = dsConnection.second;
			// skip common source net
			if (&labelDotPinList == commonSourceNets)
				continue;
			outfile << ".net ";
			for (const auto& labelDotPin : labelDotPinList)
			{
				outfile << labelDotPin << " ";
			}
			outfile << "\n";
		}

		outfile << "END_COMMON_NODE\n";

		for (const auto& gConnection : netNameMapLabelDotPinG)
		{
			const auto& netName = gConnection.first;
			const auto& labelDotPinList = gConnection.second;
			outfile << ".net ";
			for (const auto& labelDotPin : labelDotPinList)
			{
				outfile << labelDotPin << " ";
			}
			outfile << "\n";
		}
	}

	outfile << "END_NET\n";
	outfile.close();
	return true;
}