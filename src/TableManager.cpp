#include <vector>

#include "TableManager.h"

using namespace std;

TableManager::TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist)
{
	this->groupSize = groupSize;
	this->rowSize = rowSize;
	this->colSize = colSize;
	this->netlist = netlist;
	InitializeTable();
}

void TableManager::InitializeTable()
{
	table.resize(rowSize, std::vector<Group>(colSize));
}

// output part
std::vector<std::string> TableManager::GetTableStringFormat()
{
	std::vector<std::string> tableStrings;
	
	for (auto& row : table)
	{
		std::string rowString;
		for (auto& group : row)
		{
			for (const auto& deviceUnit : group.GetDeviceUnits())
			{
				rowString += deviceUnit.GetSymbol();
			}
		}
		tableStrings.push_back(rowString);
	}

	return tableStrings;
}

std::vector<std::string> TableManager::GetTableRotationFormat()
{
	std::vector<std::string> tableRotations;
	for (auto& row : table)
	{
		std::string rowRotation;
		for (auto& group : row)
		{
			for (const auto& deviceUnit : group.GetDeviceUnits())
			{
				rowRotation += std::to_string(static_cast<int>(deviceUnit.GetRotation()));
			}
		}
		tableRotations.push_back(rowRotation);
	}
	return tableRotations;
}


// cost main function
std::unordered_map<CostEnum, double> TableManager::CalculateTableCost()
{
	this->costMap.clear();
	this->CalculateCCCost();
	this->CalculateRCost();
	this->CalculateCCost();
	this->CalculateSpetationCost();
	return this->costMap;
}

// cost part implementation
void TableManager::CalculateCCCost() { ; }
void TableManager::CalculateRCost() { ; }
void TableManager::CalculateCCost() { ; }
void TableManager::CalculateSpetationCost() { ; }


// check colume rule(same type sequential)
bool TableManager::ColumnRuleCheck(int rowPlace, int colPlace, Group& group)
{
	int nowGroupTypeHash = group.GetTypeHash();

	// check above
	if (rowPlace - 1 >= 0)
	{
		int aboveGroupTypeHash = table[rowPlace - 1][colPlace].GetTypeHash();
		if (nowGroupTypeHash == aboveGroupTypeHash)
		{
			return false;
		}
	}

	// check below
	if (rowPlace + 1 < rowSize)
	{
		int belowGroupTypeHash = table[rowPlace + 1][colPlace].GetTypeHash();
		if (nowGroupTypeHash == belowGroupTypeHash)
		{
			return false;
		}
	}

	return true;
}
// check row rule(neighborhood group can link)
bool TableManager::RowRuleCheck(int rowPlace, int colPlace, Group& group)
{
	vector<DeviceUnit> groupDeviceUnits = group.GetDeviceUnits();
	
	// check left
	DeviceUnit firstDeviceUnit = groupDeviceUnits[0];
	if (colPlace - 1 >= 0)
	{
		Group leftGroup = table[rowPlace][colPlace - 1];
		pair<string, string> leftGroupLastWhoAndOuterPin = leftGroup.GetLastDeviceUnitWhoAndOuterPin();

		if (leftGroupLastWhoAndOuterPin.first != firstDeviceUnit.GetSymbol()) return false;
	}

	// check right
	DeviceUnit lastDeviceUnit = groupDeviceUnits.back();
	if (colPlace + 1 < colSize)
	{
		Group rightGroup = table[rowPlace][colPlace + 1];
		pair<string, string> rightGroupFirstWhoAndOuterPin = rightGroup.GetFirstDeviceUnitWhoAndOuterPin();
		if (rightGroupFirstWhoAndOuterPin.first != lastDeviceUnit.GetSymbol()) return false;
	}

	return true;
}


// placement operations implementation
bool TableManager::PlaceGroup(const Group& group, int& placedRow, int& placedCol)
{
	return false;
}
bool TableManager::SwapGroups(int row1, int col1, int row2, int col2)
{
	return false;
}
bool TableManager::MoveGroup(int srcRow, int srcCol, int destRow, int destCol)
{
	return false;
}


bool TableManager::EqualTableToSelf(TableManager& otherTable)
{
	// first concate the table into string
	std::vector<std::string> thisTableStrings = this->GetTableStringFormat();
	std::vector<std::string> otherTableStrings = otherTable.GetTableStringFormat();

	return thisTableStrings == otherTableStrings;
}

void TableManager::SetTableSize(int rowSize, int colSize)
{
	this->rowSize = rowSize;
	this->colSize = colSize;
	InitializeTable();
}