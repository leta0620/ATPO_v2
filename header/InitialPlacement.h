#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "TableManager.h"
#include "NetListLookupTable.h"


class InitialPlacement
{
public:
	InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist, std::vector<CostEnum> costEnumList, bool busFlag);

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
	bool busFlag; // to determine whether to use bus-based group allocation or odd-even group allocation
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