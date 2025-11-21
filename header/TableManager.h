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
	std::unordered_map<CostEnum, int> CalculateTableCost();


private:
	std::vector<std::vector<Group>> table;
	int rowSize = 0;
	int colSize = 0;
	int groupSize = 0;

	void InitializeTable();
	// check colume rule(same type sequential)
	bool ColumnRuleCheck(int rowPlace, int colPlace, const Group& group);
	// check row rule(neighborhood group can link)
	bool RowRuleCheck(int rowPlace, int colPlace, const Group& group);

	// cost part
	std::unordered_map<CostEnum, int> costMap;

	void CalculateCCCost();
	void CalculateRCost();
	void CalculateCCost();
	void CalculateSpetationCost();
};