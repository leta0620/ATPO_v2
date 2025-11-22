#include <unordered_map>
#include <string>
#include <vector>

#include "NetlistUnit.h"

class NetlistLookupTable
{
public:
	std::string FindPinS(std::string synbolName);
	std::string FindPinG(std::string synbolName);
	std::string FindPinD(std::string synbolName);

	void AddNetlistUnit(const NetlistUnit& netlistUnit);
	NetlistUnit GetNetlistUnit(const std::string& synbolName);

	void SetCommonSourceList(const std::vector<std::string>& sourceList) { this->commonSourceList = sourceList; }
	std::vector<std::string> GetCommonSourceList() const { return this->commonSourceList; }



private:
	std::vector<std::string> commonSourceList;

	std::unordered_map<std::string, NetlistUnit> netlistUnitMap; // key(synbolName) -> NetlistUnit
};