#pragma once
#include <string>
#include <vector>
#include "TableManager.h"
#include "NetListLookupTable.h"


class InitialPlacement
{
public:
	InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist);

	std::vector<TableManager>& GetInitialTableList() 
	{
		return InitialTableList;
	}

private:
	NetlistLookupTable netListLookupTable;

	std::vector<TableManager> InitialTableList;
	std::vector<std::vector<DeviceUnit>> pathOrder;
	int groupSize;
	int rowSize;
	int colSize; // to be determined

	// Calculate initial placement tables list
	void CalculateInitialTableList();

	void CalculateColSize();

	void InitialPathOrder();

};