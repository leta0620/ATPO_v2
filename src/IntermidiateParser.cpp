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
		return;
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

	// setup NetListLookupTable list
	// device build
	for (size_t i = 0; i < this->deviceInstanceList.size(); ++i)
	{
		auto& deviceInstance = this->deviceInstanceList[i];
		//NetListLookupTable netListLookupTable(std::get<0>(deviceInstance), std::get<1>(deviceInstance), std::get<3>(deviceInstance), std::get<2>(deviceInstance));
		//// connections build
		//if (i < this->connectionList.size())
		//{
		//	auto& connection = this->connectionList[i];
		//	for (const auto& netName : connection)
		//	{
		//		netListLookupTable.AddS(netName);
		//	}
		//}
		//if (i < this->otherConnectionList.size())
		//{
		//	auto& otherConnection = this->otherConnectionList[i];
		//	for (const auto& netName : otherConnection)
		//	{
		//		netListLookupTable.AddD(netName);
		//	}
		//}
		//if (i < this->gateConnectionList.size())
		//{
		//	auto& gateConnection = this->gateConnectionList[i];
		//	for (const auto& netName : gateConnection)
		//	{
		//		netListLookupTable.AddG(netName);
		//	}
		//}
		//this->netListLookupTableList.push_back(netListLookupTable);
	}


	infile.close();
	return true;
}