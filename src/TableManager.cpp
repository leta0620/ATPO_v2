#include "TableManager.h"

TableManager::TableManager(int groupSize, int rowSize, int colSize)
{
	this->groupSize = groupSize;
	this->rowSize = rowSize;
	this->colSize = colSize;
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
			rowString += group.GetName();
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
bool TableManager::ColumnRuleCheck(int rowPlace, int colPlace, const Group& group)
{
	return true;
}
// check row rule(neighborhood group can link)
bool TableManager::RowRuleCheck(int rowPlace, int colPlace, const Group& group)
{
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