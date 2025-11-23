#pragma once
#include <string>
#include <vector>
#include "TableManager.h"
#include "NetListLookupTable.h"


class InitialPlacement
{
public:
	InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist) : groupSize(groupSize), rowSize(rowSize), colSize(0), netListLookupTable(netlist)
	{
		InitialTableList.resize(rowSize, TableManager(groupSize, rowSize, colSize, netListLookupTable));
	}

	std::vector<TableManager>& GetInitialTableList() 
	{
		return InitialTableList;
	}

private:
	NetlistLookupTable netListLookupTable;

	std::vector<TableManager> InitialTableList;
	int groupSize;
	int rowSize;
	int colSize; // to be determined

	// Calculate initial placement tables list
	void CalculateInitialTableList();
};