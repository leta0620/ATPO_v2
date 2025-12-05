#pragma once
#include "TableManager.h"
#include <map>
#include <vector>
#include <string>

class Output
{
public:
	Output(int group, int rowSize, std::map<int, std::vector<TableManager>>& allNondominatedSolutions, std::string sOrD) : group(group), rowSize(rowSize), allNondominatedSolutions(allNondominatedSolutions) 
	{  
		if (sOrD == "S" || sOrD == "s")
			leftS = true;
		else
			leftS = false;
	}

	void AddResultSingle(int round, TableManager& cTable);
	void AddResultList(int round, std::vector<TableManager> cTableList);

	void WriteAllResultToFile(std::string fileName);
	void PrintAllResult();

	void SelectSignificantNondominatedSolutions();

	//std::vector<TableManager> GetSignificantNondominatedSolutions() { return significantNondominatedSolutions; }
	void PrintSignificantNondominatedSolutions();
	void WriteSignificantNondominatedSolutionsToFile(std::string fileName);

private:
	std::map<int, std::vector<TableManager>> allNondominatedSolutions;

	std::map<CostEnum, std::vector<TableManager>> significantNondominatedSolutions;

	void SelectTopNByCostEnum(CostEnum costEnum, int N);

	int group;
	int rowSize;
	bool leftS = true;
};