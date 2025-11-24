#include "NetListLookupTable.h"

std::string NetlistLookupTable::GetPinSLinkWho(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		return it->second.GetPinS(synbolName);
	}
	return "";
}

std::string NetlistLookupTable::GetPinGLinkWho(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		return it->second.GetPinG(synbolName);
	}
	return "";
}

std::string NetlistLookupTable::GetPinDLinkWho(std::string synbolName)
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

std::vector<std::string> NetlistLookupTable::GetAllSymbolNames()
{
	std::vector<std::string> symbolNames;
	symbolNames.reserve(this->netlistUnitMap.size());
	for (const auto& pair : this->netlistUnitMap)
	{
		symbolNames.push_back(pair.first);
	}
	return symbolNames;
}