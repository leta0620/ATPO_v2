#pragma once
#include "TableManager.h"
#include "OuterInput.h"
#include <map>
#include <vector>
#include <string>

class Output
{
public:
	Output(int group, int rowSize, std::map<int, std::vector<TableManager>>& allNondominatedSolutions, std::string sOrD, std::unordered_map<std::string, std::string> labelNameMapInstName, std::unordered_map<std::string, std::string> instNameMapLabelName) :
		group(group), rowSize(rowSize), allNondominatedSolutions(allNondominatedSolutions), labelNameMapInstName(labelNameMapInstName), instNameMapLabelName(instNameMapLabelName)
	{  
		if (sOrD == "S" || sOrD == "s")
			leftS = true;
		else
			leftS = false;
	}

	//void AddResultSingle(int round, TableManager& cTable);
	//void AddResultList(int round, std::vector<TableManager> cTableList);

	void WriteAllResultToFile(std::string fileName);
	void PrintAllResult();


	void SelectSignificantNondominatedSolutions();
	void PrintSignificantNondominatedSolutions();
	void WriteSignificantNondominatedSolutionsToFile(std::string fileName);

	void WriteGlobalNondominatedSolutionsToFile(std::string fileName);

private:
	std::map<int, std::vector<TableManager>> allNondominatedSolutions;

	std::map<CostEnum, std::vector<TableManager>> significantNondominatedSolutions;

	std::map<int, std::vector<TableManager>> globalNondominatedSolutions;
	void GenGlobalNondominatedSolutions();

	void SelectTopNByCostEnum(CostEnum costEnum, int N);

	int group;
	int rowSize;
	bool leftS = true;

	std::unordered_map<std::string, std::string> labelNameMapInstName; // label name -> inst name
	std::unordered_map<std::string, std::string> instNameMapLabelName; // inst name -> label name
};