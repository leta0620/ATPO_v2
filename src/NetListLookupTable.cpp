#include "NetListLookupTable.h"

using namespace std;

pair<std::string, std::string> NetlistLookupTable::GetPinSLinkWho(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		//return it->second.GetPinS(synbolName);
		string who = it->second.GetPinS(synbolName);

		// return target synbol name and pin name
		auto itWho = this->netlistUnitMap.find(who);
		if (itWho != this->netlistUnitMap.end())
		{
			if (itWho->second.GetPinD(who) == synbolName)
			{
				return pair<string, string>(who, "D");
			}
			else if (itWho->second.GetPinG(who) == synbolName)
			{
				return pair<string, string>(who, "G");
			}
			else if (itWho->second.GetPinS(who) == synbolName)
			{
				return pair<string, string>(who, "S");
			}
		}
		else
		{
			return pair<string, string>(who, "");
		}
	}
	return pair<string, string>("", "");
}

pair<std::string, std::string> NetlistLookupTable::GetPinGLinkWho(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		string who = it->second.GetPinG(synbolName);

		// return target synbol name and pin name
		auto itWho = this->netlistUnitMap.find(who);
		if (itWho != this->netlistUnitMap.end())
		{
			if (itWho->second.GetPinD(who) == synbolName)
			{
				return pair<string, string>(who, "D");
			}
			else if (itWho->second.GetPinG(who) == synbolName)
			{
				return pair<string, string>(who, "G");
			}
			else if (itWho->second.GetPinS(who) == synbolName)
			{
				return pair<string, string>(who, "S");
			}
		}
		else
		{
			return pair<string, string>(who, "");
		}
	}
	return pair<string, string>("", "");
}

pair<std::string, std::string> NetlistLookupTable::GetPinDLinkWho(std::string synbolName)
{
	auto it = this->netlistUnitMap.find(synbolName);
	if (it != this->netlistUnitMap.end())
	{
		//return it->second.GetPinD(synbolName);
		string who = it->second.GetPinD(synbolName);

		// return target synbol name and pin name
		auto itWho = this->netlistUnitMap.find(who);
		if (itWho != this->netlistUnitMap.end())
		{
			if (itWho->second.GetPinD(who) == synbolName)
			{
				return pair<string, string>(who, "D");
			}
			else if (itWho->second.GetPinG(who) == synbolName)
			{
				return pair<string, string>(who, "G");
			}
			else if (itWho->second.GetPinS(who) == synbolName)
			{
				return pair<string, string>(who, "S");
			}
		}
		else
		{
			return pair<string, string>(who, "");
		}
	}
	return pair<string, string>("", "");
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
