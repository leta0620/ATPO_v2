#include "IntermidiateParser.h"

#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

bool IntermidiateParser::Parse()
{
	ifstream infile(this->intrrmediateFilePath);
	if (!infile.is_open())
	{
		cerr << "Error: Unable to open file " << this->intrrmediateFilePath << endl;
		return false;
	}

	string line;
	while (getline(infile, line))
	{
		stringstream ss(line);
		string token;
		ss >> token;

		if (token == "DEVICE_BEGIN")
		{
			while (getline(infile, line))
			{
				if (line == "END_DEVICE")
					break;


				string deviceLine;
				stringstream deviceSS(line);
				string deviceToken;
				tuple<string, string, string, int> deviceInstance;
				deviceSS >> get<0>(deviceInstance) >> get<1>(deviceInstance) >> get<2>(deviceInstance) >> get<3>(deviceInstance);
				this->deviceInstanceList.push_back(deviceInstance);
			}
		}

		if (token == "NET_BEGIN")
		{
			while (getline(infile, line))
			{
				if (line == "END_NET")
					break;

				if (line == "COMMON_NODE_BEGIN")
				{
					string commonNodeLine;
					stringstream commonNodeSS;

					while (getline(infile, commonNodeLine))
					{
						commonNodeSS = stringstream(commonNodeLine);
						if (commonNodeLine == "END_COMMON_NODE")
							break;

						string commonNodeToken;
						commonNodeSS >> commonNodeToken;
						if (commonNodeToken == ".net_S")
						{
							string cellName;
							while (commonNodeSS >> cellName)
							{
								this->commonSourceCellList.push_back(cellName);
							}
						}
						else if (commonNodeToken == ".net")
						{
							string cellName;
							vector<string> connection;
							while (commonNodeSS >> cellName)
							{
								connection.push_back(cellName);
							}
							this->connectionList.push_back(connection);
						}
						else
						{
							cout << "Unknown common node token: " << commonNodeToken << endl;
							return false;
						}
					}
				}

				while (getline(infile, line))
				{
					if (line == "END_NET")
						break;

					string gateLine;
					stringstream gateSS(line);
					string gateToken;
					gateSS >> gateToken;

					if (gateToken == ".net")
					{
						vector<string> gateConnection;
						while (gateSS >> gateToken)
						{
							gateConnection.push_back(gateToken);
						}
						this->gateConnectionList.push_back(gateConnection);
					}
				}

			}
		}
	}

	infile.close();
	return true;
}

bool IntermidiateParser::GenerateNetlistLookupTable()
{
	unordered_map<string, NetlistUnit> tempNetlistMap;
	// device 
	for (const auto& deviceInstance : this->deviceInstanceList)
	{
		// 使用 C++17 結構綁定直接取出 tuple 的各個欄位
		const auto& [cellName, synbolName, analogcellType, unitCount] = deviceInstance;

		NetlistUnit unit;
		unit.SetAnalogType(analogcellType);
		unit.SetCellName(cellName);
		unit.SetSynbolName(synbolName);
		unit.SetDeviceUnitCount(unitCount);
		tempNetlistMap[synbolName] = unit;
	}

	// build real common source device list
	vector<string> realCommonSourceList;
	realCommonSourceList.reserve(this->commonSourceCellList.size());
	for (const auto& cellName : this->commonSourceCellList)
	{
		size_t pos = cellName.find('.');
		if (pos == string::npos)
			realCommonSourceList.push_back(cellName);
		else
			realCommonSourceList.push_back(cellName.substr(0, pos));
	}
	this->netlistLookupTable.SetCommonSourceList(realCommonSourceList);

	// net connections
	for (const auto& cell : this->commonSourceCellList)
	{
		string synbolName, pinName;
		size_t pos = cell.find('.');
		if (pos == string::npos)
		{
			cerr << "Error: Invalid cell name format in common source list: " << cell << endl;
			return false;
		}
		else
		{
			synbolName = cell.substr(0, pos);
			pinName = cell.substr(pos + 1);
		}

		auto it = tempNetlistMap.find(synbolName);
		if (it != tempNetlistMap.end())
		{
			if (pinName == "S")
			{
				it->second.AddPin(synbolName, "", "", "COMMON_SOURCE");
			}
			else
			{
				cerr << "Error: Invalid pin name in common source list: " << pinName << endl;
				return false;
			}
		}
		else
		{
			cerr << "Error: Symbol name not found in device instances: " << synbolName << endl;
			return false;
		}
	}

	for (const auto& linkList : this->connectionList)
	{
		if (linkList.size() > 2)
		{
			cerr << "Error: More than two connections in other connection list." << endl;
			return false;
		}
		else if (linkList.size() == 2)
		{
			string c1 = linkList[0], c2 = linkList[1];
			string c1SynbolName, c1PinName, c2SynbolName, c2PinName;
			size_t pos1 = c1.find('.');
			size_t pos2 = c2.find('.');
			if (pos1 == string::npos || pos2 == string::npos)
			{
				cerr << "Error: Invalid cell name format in other connection list." << endl;
				return false;
			}
			else
			{
				c1SynbolName = c1.substr(0, pos1);
				c1PinName = c1.substr(pos1 + 1);
				c2SynbolName = c2.substr(0, pos2);
				c2PinName = c2.substr(pos2 + 1);
			}

			auto it = tempNetlistMap.find(c1SynbolName);
			if (it != tempNetlistMap.end())
			{
				if (c1PinName == "D")
				{
					it->second.AddPinD(c1SynbolName, c2SynbolName);
				}
				else if (c1PinName == "S")
				{
					it->second.AddPinS(c1SynbolName, c2SynbolName);
				}
				else if (c1PinName == "G")
				{
					it->second.AddPinG(c1SynbolName, c2SynbolName);
				}
				else
				{
					cerr << "Error: Invalid pin names in other connection list: " << c1PinName << ", " << c2PinName << endl;
					return false;
				}
			}
			else
			{
				cerr << "Error: Symbol name not found in device instances: " << c1SynbolName << endl;
				return false;
			}

			it = tempNetlistMap.find(c2SynbolName);
			if (it != tempNetlistMap.end())
			{
				if (c2PinName == "D")
				{
					it->second.AddPinD(c2SynbolName, c1SynbolName);
				}
				else if (c2PinName == "S")
				{
					it->second.AddPinS(c2SynbolName, c1SynbolName);
				}
				else if (c2PinName == "G")
				{
					it->second.AddPinG(c2SynbolName, c1SynbolName);
				}
				else
				{
					cerr << "Error: Invalid pin names in other connection list: " << c1PinName << ", " << c2PinName << endl;
					return false;
				}
			}
			else
			{
				cerr << "Error: Symbol name not found in device instances: " << c2SynbolName << endl;
				return false;
			}
		}
		else if (linkList.size() == 1)
		{
			// Single connection, no action needed
		}
		else
		{
			cerr << "Error: Empty connection list in other connection list." << endl;
			return false;
		}
	}
	for (const auto& deviceInstance : this->deviceInstanceList)
	{
		// 使用 C++17 結構綁定直接取出 tuple 的各個欄位
		const auto& [cellName, synbolName, analogcellType, unitCount] = deviceInstance;

		NetlistUnit unit;
		unit = tempNetlistMap[synbolName];
		this->netlistLookupTable.AddNetlistUnit(unit);
	}


	// gate link 先不處理
	/*for (const auto& linkList : this->gateConnectionList)
	{
		for (const auto& cell : linkList)
		{

		}
	}*/

	// put all device to netlistlookuptable
	for (const auto& pair : tempNetlistMap)
	{
		this->netlistLookupTable.AddNetlistUnit(pair.second);
	}

	return true;
}