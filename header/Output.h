#pragma once
#include "TableManager.h"
#include <map>
#include <vector>
#include <string>

class Output
{
public:
	Output(int group, int rowSize, std::vector<int> deviceNumList) : group(group), rowSize(rowSize), deviceNumList(deviceNumList) { ; }

	void AddResultSingle(int round, TableManager& cTable);
	void AddResultList(int round, std::vector<TableManager> cTableList);

	void WriteAllResultToFile(std::string fileName);
	void PrintAllResult();

	void SelectSignificantNondominatedSolutions();

	std::vector<TableManager> GetSignificantNondominatedSolutions() { return significantNondominatedSolutions; }
	void PrintSignificantNondominatedSolutions();
	void WriteSignificantNondominatedSolutionsToFile(std::string fileName);

private:
	std::map<int, std::vector<TableManager>> allNondominatedSolutions;

	std::vector<TableManager> significantNondominatedSolutions;

	int group;
	int rowSize;
	std::vector<int> deviceNumList;
};