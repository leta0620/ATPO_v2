#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "TableManager.h"
#include "NetListLookupTable.h"


class InitialPlacement
{
public:
	InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist, std::vector<CostEnum> costEnumList);

	std::vector<TableManager>& GetInitialTableList() 
	{
		return InitialTableList;
	}

private:
	NetlistLookupTable netListLookupTable;

	std::vector<TableManager> InitialTableList;
	std::vector<std::vector<DeviceUnit>> pathOrder;
	std::vector<std::vector<Group>> allConfigurationGroupForTables;
	int groupSize;
	int rowSize;
	int colSize; // to be determined
	std::vector<CostEnum> costEnumList;

	// Calculate initial placement tables list
	void CalculateInitialTableList();

	//void CalculateColSize();

	//void InitialPathOrder();

	void GroupAllocation();
	void regularGroupAllocation();

};