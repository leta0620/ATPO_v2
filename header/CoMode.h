#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "TableManager.h"
#include "NetListLookupTable.h"


class CoMode
{
private:
	int groupSize = 2;
	int rowSize;
	NetlistLookupTable& netlistLookupTable;
	std::vector<CostEnum> costEnumList;

	void BuildCoModeInitialTable();
	void CalculateCoModeBestTable();

	std::vector<TableManager> tableList;
	std::vector<TableManager> bestTableList;

public:
	CoMode(int groupSize, int rowSize, NetlistLookupTable& netlistLookupTable, std::vector<CostEnum> costEnumList)
		: groupSize(groupSize), rowSize(rowSize), netlistLookupTable(netlistLookupTable), costEnumList(costEnumList)
	{
		BuildCoModeInitialTable();
		CalculateCoModeBestTable();
	}	


	//std::vector<TableManager> GetInitialTableList() const { return initialTableList; }
	std::vector<TableManager> GetBestTableList() const { return bestTableList; }
	std::vector<TableManager> GetTableList() const { return tableList; }
};