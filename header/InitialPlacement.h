#include <string>
#include <vector>
#include "TableManager.h"


class InitialPlacement
{
public:
	InitialPlacement(int groupSize, int rowSize, int colSize) : groupSize(groupSize), rowSize(rowSize), colSize(colSize) 
	{
		InitialTableList.resize(rowSize, TableManager(groupSize, rowSize, colSize));
	}

	std::vector<TableManager>& GetInitialTableList() 
	{
		return InitialTableList;
	}

private:
	std::vector<TableManager> InitialTableList;
	int groupSize;
	int rowSize;
	int colSize;

	// Calculate initial placement tables list
	void CalculateInitialTableList();
};