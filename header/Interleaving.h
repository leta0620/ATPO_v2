#include "TableManager.h"

class Interleaving
{
public:
	void Interleave(TableManager& tableManager, NetlistLookupTable& netlist);

private:
	std::string circuitType;

	std::vector<TableManager> nondominatedSolution;
	TableManager initialTable;
	TableManager nowUseTable;
	std::vector<TableManager> newTableList;

	NetlistLookupTable netlistLookupTable;

	vector<Group> groupVec;

	void InterleaveProcess();
};