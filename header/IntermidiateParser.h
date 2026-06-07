#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "NetListLookupTable.h"
#include "NetlistUnit.h"

class IntermidiateParser
{
public:
	IntermidiateParser(const std::string& intermidiateFilePath)
		: intrrmediateFilePath(intermidiateFilePath)
	{ ;	}

	bool Parse();
	bool GenerateNetlistLookupTable();

	NetlistLookupTable& GetNetlistLookupTable() { return this->netlistLookupTable; }

	std::vector<std::string> GetCommonSourceCellList() const { return this->commonSourceCellList; }

	bool GetAllDeviceOnlyOneUnitFlag() const { return this->AllDeviceOnlyOneUnitFlag; }
	
private:
	std::string intrrmediateFilePath;

	std::vector<std::string> commonSourceCellList;
	NetlistLookupTable netlistLookupTable;
	void CheckAllDeviceOnlyOneUnit();

	bool commonSourceFlag = false; // if true, means there is common source case, so the commonSourceCellList is valid and can be used
	bool AllDeviceOnlyOneUnitFlag = false; // if true, means all device only have one unit, so the group allocation can be simplified, and some cost can be simplified as well
	// Device instances tmp data
	std::vector<std::tuple< std::string, std::string, std::string, int, int>> deviceInstanceList; // cellName, synbolName, analogcellType, unitCount), Device instances tmp data. e.g. MM0 A analogcell 720
	std::vector<std::vector<std::string>> connectionList; // each connection is a list of net names, Device instances tmp data
	std::vector<std::vector<std::string>> otherConnectionList; // each connection is a list of net names, Device instances tmp data (S¡BD)
	std::vector<std::vector<std::string>> gateConnectionList; // each connection is a list of net names, Device instances tmp data (G)
};