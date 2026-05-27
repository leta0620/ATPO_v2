#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "Group.h"
#include "NetListLookupTable.h"

enum class CostEnum
{
	ccCost,
	rCost,
	cCost,
	sperationCost,
	dummyCost,
	routing_lengthCost,
	mildCost,// RMST routing complexity
	congestionCost, // trunk-based congestion metric (from cCost Part1)
	windowSizeCost,
	symmetryCost,
	hierCongestionCost, // hierarchical routing congestion metric
	hierCCost // cCost with hierarchical congestion replacing trunk-based
};

class TableManager
{
public:
	TableManager() = default;
	//TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist);
	TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist, std::vector<CostEnum> costEnumList);

	void SetNetlistLookupTable(NetlistLookupTable& netlist) { this->netlist = netlist; }
	void SetCdlFilePath(const std::string& path) { cdlFilePath = path; }  //3/8
	// **this function will re-initialize the table, make sure the old data is not needed**
	void SetTableSize(int rowSize, int colSize);
	void SetGroupSize(int groupSize) { this->groupSize = groupSize; }
	int GetRowSize() const { return rowSize; }
	int GetColSize() const { return colSize; }
	int GetGroupSize() const { return groupSize; }
	void SetNFin(int nfin) { this->nfin = nfin; }
	void SetCostEnumList(std::vector<CostEnum> costEnumList) { this->costEnumList = costEnumList; }

	Group GetGroup(int row, int col) const { return table[row][col]; }


	// placement operations
	bool PlaceGroup(const Group& group, int& placedRow, int& placedCol);
	bool SwapGroups(int row1, int col1, int row2, int col2);
	bool MoveGroup(int srcRow, int srcCol, int destRow, int destCol);	// caution: the src position will be cleared
	bool SwapColumns(int col1, int col2);
	bool SwapRows(int row1, int row2);

	bool CheckCanSwapGroups(int row1, int col1, int row2, int col2);

	//output
	std::vector<std::string> GetTableStringFormat();
	std::vector<std::string> GetTableRotationFormat(bool leftS = true);

	std::vector<std::string> GetTableStringPattern();
	std::vector<std::string> GetTableRotationPattern(bool leftS = true);

	//****** need to modify pattern output to real dummy unit ******
	std::vector<std::string> GetTableStringPatternInRealDummyLengthUseStartDummy();
	std::vector<std::string> GetTableStringPatternInRealDummyLength();
	std::vector<std::string> GetTableRotationPatternInRealDummyLength(bool leftS = true);

	// cost
	std::unordered_map<CostEnum, double> CalculateTableCost();
	std::unordered_map<CostEnum, double> GetCostMap() { return costMap; }
	std::vector<std::pair<std::string, double>> GetCostNameAndCostValueString();

	bool EqualTableToSelf(TableManager& otherTable);

	void PrintTableToConsole();

	// dummy width check and fix
	bool CheckAndFixDummyWidth();

	std::string GetCostName(CostEnum costEnum);

	// modify table to make it interleaving, return true if success, false if fail (if fail, the table will not be modified)
	bool BuildInterleavingTable();
	std::vector<TableManager> BuildAllInterleavingTable();
	std::vector<TableManager> BuildAllCCTable();

	void FlipLeftHalf();
	void FlipRightHalf();

private:
	std::vector<std::vector<Group>> table;
	int rowSize = 0;
	int colSize = 0;
	int groupSize = 0;

	int nfin = 4; // default 4 fin number per device unit
	int minDummyWidthInUnit = 1; // default min dummy width

	NetlistLookupTable netlist;
	std::string cdlFilePath;   // NEW 3/8

	void InitializeTable();
	// check colume rule(same type sequential)
	bool ColumnRuleCheck(int rowPlace, int colPlace, Group& group);
	// check row rule(neighborhood group can link)
	bool RowRuleCheck(int rowPlace, int colPlace, Group& group);

	std::vector<CostEnum> costEnumList = { CostEnum::ccCost, CostEnum::rCost, CostEnum::cCost, CostEnum::sperationCost, CostEnum::dummyCost, CostEnum::routing_lengthCost, CostEnum::mildCost, CostEnum::congestionCost, CostEnum::hierCongestionCost, CostEnum::hierCCost };


public:
	// cost part
	std::unordered_map<CostEnum, double> costMap;
	// Routing length switch: 0 = skip in dominance/perturbation, 1 = include.
	// Static so free functions (e.g. doesADominateB) can query without an instance.
	// Value is still always computed and written to costMap for reporting.
	static int routingLengthEnable;
	static void SetRoutingLengthEnable(int v) { routingLengthEnable = (v != 0) ? 1 : 0; }
	static int  GetRoutingLengthEnable() { return routingLengthEnable; }

private:
	void CalculateCCCost();
	void CalculateRCost();
	void CalculateCCost();
	void CalculateSpetationCost();
	void CalculateDummyCost();
	void CalculateRoutinglength();//k
	void CalculateMILDCost();
	void CalculateCongestionCost();
	void CalculateHierCongestionCost();
	void CalculateHierCCost();
	void CalculateWindowSizeCost();
	void CalculateSymmetryCost();
};