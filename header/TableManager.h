#include <vector>
#include <string>
#include <unordered_map>
#include "Group.h"

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
	TableManager(int groupSize, int rowSize, int colSize);

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