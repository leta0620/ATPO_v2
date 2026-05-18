#pragma once
#include "TableManager.h"
#include "OuterInput.h"
#include <map>
#include <vector>
#include <string>

class Output
{
public:
	Output(int group, int rowSize, std::map<int, std::vector<TableManager>>& allNondominatedSolutions, std::string sOrD, std::unordered_map<std::string, std::string> labelNameMapInstName, std::unordered_map<std::string, std::string> instNameMapLabelName, bool flipLeftHalf) :
		group(group), rowSize(rowSize), allNondominatedSolutions(allNondominatedSolutions), labelNameMapInstName(labelNameMapInstName), instNameMapLabelName(instNameMapLabelName), flipLeftHalf(flipLeftHalf)
	{  
		if (sOrD == "S" || sOrD == "s")
			leftS = true;
		else
			leftS = false;

		if (flipLeftHalf)
		{
			FlipLeftHalfOfTable();
		}
	}

	//void AddResultSingle(int round, TableManager& cTable);
	//void AddResultList(int round, std::vector<TableManager> cTableList);

	void WriteAllResultToFile(std::string fileName);
	void PrintAllResult();


	void SelectSignificantNondominatedSolutions();
	void PrintSignificantNondominatedSolutions();
	void WriteSignificantNondominatedSolutionsToFile(std::string fileName);

	void WriteGlobalNondominatedSolutionsToFile(std::string fileName);
	void PrintGlobalNondominatedSolutions();

	void WriteCSVCoBetterSolutionToFile(int topN, std::string fileName);
	void PrintCoBetterSolution(int topN);
	void WriteCoBetterSolutionToFile(int topN, std::string fileName);

	void WriteCSVCoBetterSolutionPartitionByGroupSizeToFile(int topN, std::string fileName);

	// select corporation better solution, return a list of pair of table and its coverage (coverage is represented as a pair of integers, the larger the better)
	void SelectCoBetterSolution();

	std::vector<std::pair<TableManager, std::pair<int, int>>> GetCoBetterSolutions() { return coTableScoreList; }

private:
	std::map<int, std::vector<TableManager>> allNondominatedSolutions;

	std::map<CostEnum, std::vector<TableManager>> significantNondominatedSolutions;

	std::map<int, std::vector<TableManager>> globalNondominatedSolutions;
	void GenGlobalNondominatedSolutions();

	void SelectTopNByCostEnum(CostEnum costEnum, int N);

	int group;
	int rowSize;
	bool leftS = true;
	bool flipLeftHalf = false;

	void FlipLeftHalfOfTable();

	std::vector<std::pair<TableManager, std::pair<int, int>>> coTableScoreList;

	std::unordered_map<std::string, std::string> labelNameMapInstName; // label name -> inst name
	std::unordered_map<std::string, std::string> instNameMapLabelName; // inst name -> label name
};