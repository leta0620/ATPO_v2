#include <string>
#include <vector>
#include <unordered_map>

#include "NetListLookupTable.h"

class IntermidiateParser
{
public:
	IntermidiateParser(const std::string& intermidiateFilePath)
		: intrrmediateFilePath(intermidiateFilePath)
	{ ;	}

	bool Parse();

	std::vector<std::string> GetCommonSourceCellList() const { return this->commonSourceCellList; }
	std::vector<NetListLookupTable> GetNetListLookupTableList() const { return this->netListLookupTableList; }


private:
	std::string intrrmediateFilePath;

	std::vector<std::string> commonSourceCellList;
	std::vector<NetListLookupTable> netListLookupTableList;

	// Device instances tmp data
	std::vector<tuple< std::string, std::string, std::string, int>> deviceInstanceList; // cellName, synbolName, analogcellType, unitCount), Device instances tmp data. e.g. MM0 A analogcell 720
	std::vector<std::vector<std::string>> connectionList; // each connection is a list of net names, Device instances tmp data
	std::vector<std::vector<std::string>> otherConnectionList; // each connection is a list of net names, Device instances tmp data (S¡BD)
	std::vector<std::vector<std::string>> gateConnectionList; // each connection is a list of net names, Device instances tmp data (G)
};