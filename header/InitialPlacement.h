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
	//std::vector<std::vector<DeviceUnit>> pathOrder;
	std::vector<std::vector<Group>> allConfigurationGroupForTables;
	std::vector<std::vector<Group>> allOddFirstGroupForTables;
	std::vector<std::vector<Group>> allOddSecondGroupForTables;
	int groupSize;
	int rowSize;
	int colSize; // to be determined
	std::vector<CostEnum> costEnumList;

	// Calculate initial placement tables list
	void CalculateInitialTableList();
	void CalculateOddTableList();

	//void CalculateColSize();

	//void InitialPathOrder();

	void GroupAllocation();
	void BusGroupAllocation();
	void oddGroupAllocation();
	void regularGroupAllocation();

};