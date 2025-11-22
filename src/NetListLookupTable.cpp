#include "NetlistLookupTable.h"

std::string NetlistLookupTable::FindPinS(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		return it->second.GetPinS(synbolName);
	}
	return "";
}

std::string NetlistLookupTable::FindPinG(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		return it->second.GetPinG(synbolName);
	}
	return "";
}

std::string NetlistLookupTable::FindPinD(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		return it->second.GetPinD(synbolName);
	}
	return "";
}

void NetlistLookupTable::AddNetlistUnit(const NetlistUnit& netlistUnit)
{
	this->netlistUnitMap[netlistUnit.GetSynbolName()] = netlistUnit;
}

NetlistUnit NetlistLookupTable::GetNetlistUnit(const std::string& synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		return it->second;
	}
	return NetlistUnit();
}
