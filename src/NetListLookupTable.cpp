#include <string>
#include <vector>
#include <unordered_map>
#include "NetListLookupTable.h"

using namespace std;

void NetListLookupTable::AddS(std::string s)
{
	for (auto& [key, terminals] : this->lookupTable)
	{
		std::get<2>(terminals) = s;
	}
}

void NetListLookupTable::AddG(std::string g)
{
	for (auto& [key, terminals] : this->lookupTable)
	{
		std::get<1>(terminals) = g;
	}
}

void NetListLookupTable::AddD(std::string d)
{
	for (auto& [key, terminals] : this->lookupTable)
	{
		std::get<0>(terminals) = d;
	}
}

tuple<std::string, std::string, std::string> NetListLookupTable::GetTerminals(const std::string& key) const
{
	auto it = this->lookupTable.find(key);
	if (it != this->lookupTable.end())
	{
		return it->second;
	}
	else
	{
		return make_tuple("", "", "");
	}
}