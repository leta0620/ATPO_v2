#include <vector>
#include <string>
#include <unordered_map>
#include "Group.h"
#include "NetlistLookupTable.h"

enum class CostEnum
{
	ccCost,
	rCost,
	cCost,
	spetationCost
};

class TableManager
{
public:
	TableManager() = default;
	TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist);

	void SetNetlistLookupTable(NetlistLookupTable& netlist) { this->netlist = netlist; }

	// **this function will re-initialize the table, make sure the old data is not needed**
	void SetTableSize(int rowSize, int colSize);
	void SetGroupSize(int groupSize) { this->groupSize = groupSize; }
	int GetRowSize() const { return rowSize; }
	int GetColSize() const { return colSize; }
	int GetGroupSize() const { return groupSize; }

	Group GetGroup(int row, int col) const { return table[row][col]; }


	// placement operations
	bool PlaceGroup(const Group& group, int& placedRow, int& placedCol);
	bool SwapGroups(int row1, int col1, int row2, int col2);
	bool MoveGroup(int srcRow, int srcCol, int destRow, int destCol);

	//output
	std::vector<std::string> GetTableStringFormat();
	std::vector<std::string> GetTableRotationFormat();

	// cost
	std::unordered_map<CostEnum, double> CalculateTableCost();

	std::unordered_map<CostEnum, double> GetCostMap() { return costMap; }

	bool EqualTableToSelf(TableManager& otherTable);

	void SetNFin(int nfin) { this->nfin = nfin; }


private:
	std::vector<std::vector<Group>> table;
	int rowSize = 0;
	int colSize = 0;
	int groupSize = 0;

	int nfin = 4; // default 4 fin number per device unit

	NetlistLookupTable netlist;

	void InitializeTable();
	// check colume rule(same type sequential)
	bool ColumnRuleCheck(int rowPlace, int colPlace, const Group& group);
	// check row rule(neighborhood group can link)
	bool RowRuleCheck(int rowPlace, int colPlace, const Group& group);

	// cost part
	std::unordered_map<CostEnum, double> costMap;

	void CalculateCCCost();
	void CalculateRCost();
	void CalculateCCost();
	void CalculateSpetationCost();
};