#include "Output.h"
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <set>
#include <limits>
#include <iostream>
#include <algorithm>
#include <unordered_map>
using namespace std; 

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

//void Output::AddResultSingle(int round, TableManager& cTable)
//{
//	this->allNondominatedSolutions[round].push_back(cTable);
//}
//
//void Output::AddResultList(int round, std::vector<TableManager> cTableList)
//{
//	this->allNondominatedSolutions[round] = cTableList;
//}

void Output::WriteAllResultToFile(std::string fileName)
{
	ofstream outFile(fileName);
	if (!outFile.is_open())
	{
		cerr << "Error opening file: " << fileName << endl;
		return;
	}

	for (auto& [round, tableList] : allNondominatedSolutions)
	{
		outFile << "Round " << round << " Results:\n";

		for (size_t i = 0; i < tableList.size(); ++i)
		{
			outFile << "Table " << i + 1 << ":\n";
			outFile << "group size: " << tableList[i].GetGroupSize() << "\n";

			// 遍歷cost並輸出，要有項目名稱和數值
			auto costNameAndValue = tableList[i].GetCostNameAndCostValueString();
			for (auto cost : costNameAndValue)
			{
				outFile << cost.first << ":" << cost.second << "\t";
			}
			outFile << "\n";

			auto tableStrings = tableList[i].GetTableStringFormat();
			auto rotationStrings = tableList[i].GetTableRotationFormat(leftS);
			for (const auto& rowString : tableStrings)
			{
				outFile << rowString << "\n";
			}

			outFile << "Rotations:\n";
			for (const auto& rotationString : rotationStrings)
			{
				outFile << rotationString << "\n";
			}
			outFile << "\n";
		}
		outFile << "----------------------------------------\n";
	}
	outFile.close();
}

void Output::PrintAllResult()
{
	for (auto& [round, tableList] : allNondominatedSolutions)
	{
		cout << "Round " << round << " Results:\n";
		for (size_t i = 0; i < tableList.size(); ++i)
		{
			cout << "Table " << i + 1 << ":\n";
			cout << "group size: " << tableList[i].GetGroupSize() << "\n";
			auto costNameAndValue = tableList[i].GetCostNameAndCostValueString();
			for (auto cost : costNameAndValue)
			{
				cout << cost.first << ":" << cost.second << "\t";
			}
			cout << "\n";

			auto tableStrings = tableList[i].GetTableStringFormat();
			auto rotationStrings = tableList[i].GetTableRotationFormat(leftS);
			for (const auto& rowString : tableStrings)
			{
				cout << rowString << "\n";
			}
			cout << "Rotations:\n";
			for (const auto& rotationString : rotationStrings)
			{
				cout << rotationString << "\n";
			}
			cout << "\n";
		}
		cout << "----------------------------------------\n";
	}
}


void Output::SelectSignificantNondominatedSolutions()
{
	// TO DO: implement selection logic

	// select top 5 significant solutions by each cost enum
	for (int costEnumInt = 0; costEnumInt < 10; ++costEnumInt)
	{
		CostEnum costEnum = static_cast<CostEnum>(costEnumInt);
		SelectTopNByCostEnum(costEnum, 5);
	}
}

void Output::PrintSignificantNondominatedSolutions()
{

	for (auto& sol : significantNondominatedSolutions)
	{
		if (sol.second.empty())
		{
			cout << "No significant nondominated solutions selected for this cost enum : " << static_cast<int>(sol.first) << "\n";
			continue;
		}
		else
		{
			std::string costItem = sol.second[0].GetCostName(sol.first);
			cout << "Cost Item : " << costItem << "\n";
		}

		for (size_t i = 0; i < sol.second.size(); ++i)
		{
			cout << "Significant Nondominated Solution first " << 10 + i*20 << "% :\n";
			cout << "group size: " << sol.second[i].GetGroupSize() << "\n";
			auto costNameAndValue = sol.second[i].GetCostNameAndCostValueString();
			for (auto cost : costNameAndValue)
			{
				cout << cost.first << ":" << cost.second << "\n";
			}
			cout << "\n";

			auto tableStrings = sol.second[i].GetTableStringFormat();
			auto rotationStrings = sol.second[i].GetTableRotationFormat(leftS);
			for (const auto& rowString : tableStrings)
			{
				cout << rowString << "\n";
			}
			cout << "Rotations:\n";
			for (const auto& rotationString : rotationStrings)
			{
				cout << rotationString << "\n";
			}
			cout << "\n";
		}
	}
}

void Output::WriteSignificantNondominatedSolutionsToFile(std::string fileName)
{
	ofstream outFile(fileName);
	if (!outFile.is_open())
	{
		cerr << "Error opening file: " << fileName << endl;
		return;
	}

	for (auto& sol : significantNondominatedSolutions)
	{
		string costItem;
		if (sol.second.empty())
		{
			outFile << "No significant nondominated solutions selected for this cost enum : " << static_cast<int>(sol.first) << "\n";
			continue;
		}
		else
		{
			costItem = sol.second[0].GetCostName(sol.first);
			outFile << "Cost Item : " << costItem << "\n";
		}

		for (size_t i = 0; i < sol.second.size(); ++i)
		{
			outFile << "Significant Nondominated Solution first " << 10 + i * 20 << "% :\n";
			outFile << "group size: " << sol.second[i].GetGroupSize() << "\n";
			auto costNameAndValue = sol.second[i].GetCostNameAndCostValueString();
			for (auto cost : costNameAndValue)
			{
				outFile << cost.first << ":" << cost.second << "\t";
			}
			outFile << "\n";

			auto tableStrings = sol.second[i].GetTableStringFormat();
			auto rotationStrings = sol.second[i].GetTableRotationFormat(leftS);
			for (const auto& rowString : tableStrings)
			{
				outFile << rowString << "\n";
			}
			outFile << "Rotations:\n";
			for (const auto& rotationString : rotationStrings)
			{
				outFile << rotationString << "\n";
			}
			outFile << "\n";

			// output pattern file
			string patternFileName = fileName + "_" + costItem + "_Solution_" + to_string(i + 1) + ".csv";
			//vector<string> pattern = sol.second[i].GetTableStringPattern();
			//vector<string> rotationPattern = sol.second[i].GetTableRotationPattern(leftS);
			vector<string> pattern = sol.second[i].GetTableStringPatternInRealDummyLength();
			vector<string> rotationPattern = sol.second[i].GetTableRotationPatternInRealDummyLength(leftS);


			ofstream patternOutFile(patternFileName);

			if (!patternOutFile.is_open())
			{
				cerr << "Error opening pattern file: " << patternFileName << endl;
				continue;
			}

			for (const auto& patternLine : pattern)
			{
				patternOutFile << patternLine << "\n";
			}
			patternOutFile << "\n";
			for (const auto& rotationLine : rotationPattern)
			{
				patternOutFile << rotationLine << "\n";
			}
			patternOutFile << "\n";

			for (auto& [instName, labelName] : instNameMapLabelName)
			{
				patternOutFile << instName << ", " << labelName << "\n";
			}
			patternOutFile << "\n";

			patternOutFile.close();
		}
	}

	outFile.close();
}

void Output::WriteGlobalNondominatedSolutionsToFile(std::string fileName)
{
	GenGlobalNondominatedSolutions();
	ofstream outFile(fileName);
	if (!outFile.is_open())
	{
		cerr << "Error opening file: " << fileName << endl;
		return;
	}
	for (auto& [round, tableList] : globalNondominatedSolutions)
	{
		outFile << "Round " << round << " Global Nondominated Results:\n";
		for (size_t i = 0; i < tableList.size(); ++i)
		{
			outFile << "Table " << i + 1 << ":\n";
			outFile << "group size: " << tableList[i].GetGroupSize() << "\n";
			// 遍歷cost並輸出，要有項目名稱和數值
			auto costNameAndValue = tableList[i].GetCostNameAndCostValueString();
			for (auto cost : costNameAndValue)
			{
				outFile << cost.first << ":" << cost.second << "\t";
			}
			outFile << "\n";
			auto tableStrings = tableList[i].GetTableStringFormat();
			auto rotationStrings = tableList[i].GetTableRotationFormat(leftS);
			for (const auto& rowString : tableStrings)
			{
				outFile << rowString << "\n";
			}
			outFile << "Rotations:\n";
			for (const auto& rotationString : rotationStrings)
			{
				outFile << rotationString << "\n";
			}
			outFile << "\n";
		}
		outFile << "----------------------------------------\n";
	}
}

void Output::GenGlobalNondominatedSolutions()
{
	// TO DO: implement global nondominated solution generation logic
	auto doesADominateB = [](const std::unordered_map<CostEnum, double>& aCost, const std::unordered_map<CostEnum, double>& bCost) -> bool {
		bool aBetterInAtLeastOne = false;
		for (size_t i = 0; i < aCost.size(); ++i)
		{
			auto it = aCost.find(static_cast<CostEnum>(i));
			if (it != aCost.end())
			{
				int aValue = it->second;
				int bValue = bCost.at(static_cast<CostEnum>(i));
				if (aValue > bValue)
				{
					return false; // a is worse in this objective
				}
				else if (aValue < bValue)
				{
					aBetterInAtLeastOne = true; // a is better in this objective
				}
			}
		}
		return aBetterInAtLeastOne;
	};

	globalNondominatedSolutions.clear();

	for (auto& [round, tableList] : allNondominatedSolutions)
	{
		for (auto& table : tableList)
		{
			bool isDominated = false;
			for (auto& [gRound, gTableList] : globalNondominatedSolutions)
			{
				for (auto& gTable : gTableList)
				{
					if (doesADominateB(gTable.GetCostMap(), table.GetCostMap()))
					{
						isDominated = true;
						break;
					}
				}
				if (isDominated)
				{
					break;
				}
			}
			if (!isDominated)
			{
				globalNondominatedSolutions[round].push_back(table);
			}
		}
	}
}

void Output::SelectTopNByCostEnum(CostEnum costEnum, int N)
{
	// TO DO: implement selection logic
	map<double, pair<TableManager, double>> costToTableMap;

	double maxCostValue = -1.0;
	double minCostValue = std::numeric_limits<double>::max();
	for (auto& [round, tableList] : allNondominatedSolutions)
	{
		for (auto& table : tableList)
		{
			auto costMap = table.GetCostMap();
			double costValue = costMap[costEnum];
			costToTableMap[costValue].first = table;

			if (costValue > maxCostValue)
			{
				maxCostValue = costValue;
			}

			if (costValue < minCostValue)
			{
				minCostValue = costValue;
			}
		}
	}

	// calculate normolized cost and store
	for (auto& [costValue, tablePair] : costToTableMap)
	{
		double normalizedCost = (costValue - minCostValue) / (maxCostValue - minCostValue);
		tablePair.second = normalizedCost;
	}

	// find 10%, 30%, 50%, 70%, 90% table
	auto FindSpecialPercentTable = [&](double percent) -> TableManager& {
		double targetNormCost = percent;
		TableManager* closestTable = nullptr;

		if (costToTableMap.empty())
		{
			throw std::runtime_error("No tables available to select from.");
			vector<TableManager> emptyList;
			return *emptyList.begin();
		}



		for (auto& [costValue, tablePair] : costToTableMap)
		{
			if (tablePair.second > targetNormCost)
			{
				closestTable = &tablePair.first;
				break;
			}
		}

		if (closestTable == nullptr)
		{
			// if not found, return the last one
			closestTable = &costToTableMap.rbegin()->second.first;
		}

		return *closestTable;
	};

	this->significantNondominatedSolutions[costEnum].push_back(FindSpecialPercentTable(0.1));
	this->significantNondominatedSolutions[costEnum].push_back(FindSpecialPercentTable(0.3));
	this->significantNondominatedSolutions[costEnum].push_back(FindSpecialPercentTable(0.5));
	this->significantNondominatedSolutions[costEnum].push_back(FindSpecialPercentTable(0.7));
	this->significantNondominatedSolutions[costEnum].push_back(FindSpecialPercentTable(0.9));

	return;
}


void Output::PrintGlobalNondominatedSolutions()
{
	for (auto& [round, tableList] : globalNondominatedSolutions)
	{
		cout << "Round " << round << " Global Nondominated Results:\n";
		for (size_t i = 0; i < tableList.size(); ++i)
		{
			cout << "Table " << i + 1 << ":\n";
			cout << "group size: " << tableList[i].GetGroupSize() << "\n";
			auto costNameAndValue = tableList[i].GetCostNameAndCostValueString();
			for (auto cost : costNameAndValue)
			{
				cout << cost.first << ":" << cost.second << "\t";
			}
			cout << "\n";
			auto tableStrings = tableList[i].GetTableStringFormat();
			auto rotationStrings = tableList[i].GetTableRotationFormat(leftS);
			for (const auto& rowString : tableStrings)
			{
				cout << rowString << "\n";
			}
			cout << "Rotations:\n";
			for (const auto& rotationString : rotationStrings)
			{
				cout << rotationString << "\n";
			}
			cout << "\n";
		}
		cout << "----------------------------------------\n";
	}
}


tuple<int, int, double> CalCoverageRate(TableManager& table, int rowWindowSize, int colWindowSize, tuple<int, int, double> bestCoverageWin)
{
	int rowSize = table.GetRowSize();
	int colSize = table.GetColSize();
	int sameGroupCount = 0;
	int allGroupCount = 0;

	for (int i = 0; i + rowWindowSize - 1 < rowSize; i+=rowWindowSize)
	{
		for (int j = 0; j + colWindowSize - 1 < colSize; j+=colWindowSize)
		{
			allGroupCount++;
			Group currentGroup = table.GetGroup(i, j);
			
			//cout << rowSize << " " << colSize << " " << rowWindowSize << " " << colWindowSize << "\n";

			bool sameGroup = true;
			// 修正邊界：只檢查當前的 Window 範圍
			for (int x = i; x < i + rowWindowSize; ++x)
			{
				for (int y = j; y < j + colWindowSize; ++y)
				{
					if (table.GetGroup(x, y).GetSymbolNameSequence() != currentGroup.GetSymbolNameSequence())
					{
						sameGroup = false;
						break;
					}
				}
				if (!sameGroup)
				{
					break;
				}
			}

			if (sameGroup)
			{
				sameGroupCount++;
			}
		}
	}

	//if (sameGroupCount == allGroupCount)
	//{
	//	return make_tuple(rowWindowSize, colWindowSize, 1.0);
	//}

	double coverageRate = (double)sameGroupCount / allGroupCount;
	int bestRowWindowSize = rowWindowSize, bestColWindowSize = colWindowSize;


	if (rowWindowSize > 1)
	{
		tuple<int, int, double> newCoverageRate = CalCoverageRate(table, rowWindowSize / 2, colWindowSize, bestCoverageWin);
		if (get<2>(newCoverageRate) > coverageRate || ((get<2>(newCoverageRate) == coverageRate) && (get<0>(newCoverageRate) * get<1>(newCoverageRate) > bestRowWindowSize * bestColWindowSize)))
		{
			coverageRate = get<2>(newCoverageRate);
			bestRowWindowSize = get<0>(newCoverageRate);
			bestColWindowSize = get<1>(newCoverageRate);
		}
	}

	if (colWindowSize > 1)
	{
		tuple<int, int, double> newCoverageRate = CalCoverageRate(table, rowWindowSize, colWindowSize / 2, bestCoverageWin);
		if (get<2>(newCoverageRate) > coverageRate || ((get<2>(newCoverageRate) == coverageRate) && (get<0>(newCoverageRate) * get<1>(newCoverageRate) > bestRowWindowSize * bestColWindowSize)))
		{
			coverageRate = get<2>(newCoverageRate);
			bestRowWindowSize = get<0>(newCoverageRate);
			bestColWindowSize = get<1>(newCoverageRate);
		}
	}

	return make_tuple(bestRowWindowSize, bestColWindowSize, coverageRate);
}

void Output::SelectCoBetterSolution()
{
	vector<pair<TableManager, pair<int, int>>> tableScoreList;

	for (auto& [round, tableList] : allNondominatedSolutions)
	{
		for (auto& table : tableList)
		{
			tuple<int, int, double> bestCoverageRate = CalCoverageRate(table, table.GetRowSize(), table.GetColSize(), make_tuple(0, 0, 0.0));
			tableScoreList.push_back(make_pair(table, make_pair(get<0>(bestCoverageRate), get<1>(bestCoverageRate))));
		}
	}

	sort(tableScoreList.begin(), tableScoreList.end(), [](pair<TableManager, pair<int, int>>& a, pair<TableManager, pair<int, int>>& b) {
		if (a.first.GetColSize() * a.first.GetGroupSize() > b.first.GetColSize() * b.first.GetGroupSize())
		{
			return false;
		}
		else if (a.first.GetColSize() * a.first.GetGroupSize() < b.first.GetColSize() * b.first.GetGroupSize())
		{
			return true;
		}
		double aWinSize = (double)a.second.first * a.second.second * a.first.GetGroupSize();
		double bWinSize = (double)b.second.first * b.second.second * b.first.GetGroupSize();

		if (aWinSize == bWinSize)
		{
			unordered_map<CostEnum, double> costA = a.first.GetCostMap();
			unordered_map<CostEnum, double> costB = b.first.GetCostMap();

			return (costA[CostEnum::cCost] < costB[CostEnum::cCost]) || 
				(costA[CostEnum::cCost] == costB[CostEnum::cCost] && costA[CostEnum::rCost] < costB[CostEnum::rCost]) ||
				(costA[CostEnum::cCost] == costB[CostEnum::cCost] && costA[CostEnum::rCost] == costB[CostEnum::rCost] && costA[CostEnum::sperationCost] < costB[CostEnum::sperationCost]) ||
				(costA[CostEnum::cCost] == costB[CostEnum::cCost] && costA[CostEnum::rCost] == costB[CostEnum::rCost] && costA[CostEnum::sperationCost] == costB[CostEnum::sperationCost] && costA[CostEnum::mildCost] < costB[CostEnum::mildCost]);
		}


		return aWinSize > bWinSize; // sort in descending order of coverage
		});	

	this->coTableScoreList = tableScoreList;
}

void Output::PrintCoBetterSolution(int topN)
{
	for (size_t i = 0; i < coTableScoreList.size() && i < topN; ++i)
	{
		cout << "Best Solution " << i + 1 << ":\n";
		cout << "group size: " << coTableScoreList[i].first.GetGroupSize() << ", e.g. [" << coTableScoreList[i].first.GetGroup(0, 0).GetSymbolNameSequence() << "]" << endl;
		//auto costNameAndValue = coTableScoreList[i].first.GetCostNameAndCostValueString();
		//for (auto cost : costNameAndValue)
		//{
		//	cout << cost.first << ":" << cost.second << "\t";
		//}
		//cout << "\n";
		cout << "Best window size: " << coTableScoreList[i].second.first << " x " << coTableScoreList[i].second.second << "\n";

		auto tableStrings = coTableScoreList[i].first.GetTableStringFormat();
		auto rotationStrings = coTableScoreList[i].first.GetTableRotationFormat(leftS);
		
		int rowSpace = coTableScoreList[i].second.second * coTableScoreList[i].first.GetGroupSize();
		int colSpace = coTableScoreList[i].second.first;
		for (const auto& rowString : tableStrings)
		{
			for (size_t j = 0; j < rowString.size(); ++j)
			{
				cout << rowString[j];
				if ((j + 1) % rowSpace == 0)
				{
					cout << " "; // add space after each window
				}
			}
			if ((&rowString - &tableStrings[0] + 1) % colSpace == 0)
			{
				cout << "\n"; // add extra space after each column of windows
			}
			cout << "\n";
		}
		//cout << "Rotations:\n";
		//for (const auto& rotationString : rotationStrings)
		//{
		//	cout << rotationString << "\n";
		//}
		cout << "\n";
	}
}



void Output::WriteCSVCoBetterSolutionToFile(int topN, std::string fileName)
{
	int index = 1;
	fileName += "All_Best_Solutions";
	MKDIR(fileName.c_str());
	for (auto& tableScore : coTableScoreList)
	{
		if (index > topN)
		{
			break;
		}

		// output pattern file
		string patternFileName = fileName + "/Best_Solution_" + to_string(index) + ".csv";
		ofstream outFile(patternFileName);
		if (!outFile.is_open())
		{
			cerr << "Error opening file: " << patternFileName << endl;
			return;
		}

		vector<string> pattern = tableScore.first.GetTableStringPatternInRealDummyLength();
		vector<string> rotationPattern = tableScore.first.GetTableRotationPatternInRealDummyLength(leftS);

		ofstream patternOutFile(patternFileName);

		if (!patternOutFile.is_open())
		{
			cerr << "Error opening pattern file: " << patternFileName << endl;
			continue;
		}

		for (const auto& patternLine : pattern)
		{
			patternOutFile << patternLine << "\n";
		}
		patternOutFile << "\n";
		for (const auto& rotationLine : rotationPattern)
		{
			patternOutFile << rotationLine << "\n";
		}
		patternOutFile << "\n";

		for (auto& [instName, labelName] : instNameMapLabelName)
		{
			patternOutFile << instName << ", " << labelName << "\n";
		}
		patternOutFile << "\n";

		patternOutFile.close();
		
		
		index++;
	}
}

// the solution of eachfile needs to classify by group size and put in different files, the file name will be fileName + "_GroupSize_" + groupSize + ".txt"
// the file type is equal toWrite WriteSignificantNondominatedSolutionsToFile
void Output::WriteCSVCoBetterSolutionPartitionByGroupSizeToFile(int topN, std::string fileName)
{
	int overallIndex = 1;
	map<int, vector<pair<int, pair<TableManager, pair<int, int>>>>> groupSizeToTableScoreListMap;
	for (auto& tableScore : coTableScoreList)
	{
		if (overallIndex > topN)
		{
			break;
		}

		int groupSize = tableScore.first.GetGroupSize();
		groupSizeToTableScoreListMap[groupSize].push_back({overallIndex, tableScore});
		overallIndex++;
	}

	fileName += "best_solution";
	for (auto& [groupSize, tableScoreList] : groupSizeToTableScoreListMap)
	{
		string groupFileName = fileName + "_GroupSize_" + to_string(groupSize);
		MKDIR(groupFileName.c_str());
		for (auto& item : tableScoreList)
		{
			int rankIndex = item.first;
			auto& tableScore = item.second;

			string patternFileName = groupFileName + "/Best_Solution_" + to_string(rankIndex) + ".csv";
			ofstream outFile(patternFileName);
			if (!outFile.is_open())
			{
				cerr << "Error opening file: " << patternFileName << endl;
				return;
			}
			vector<string> pattern = tableScore.first.GetTableStringPatternInRealDummyLength();
			vector<string> rotationPattern = tableScore.first.GetTableRotationPatternInRealDummyLength(leftS);
			ofstream patternOutFile(patternFileName);
			if (!patternOutFile.is_open())
			{
				cerr << "Error opening pattern file: " << patternFileName << endl;
				continue;
			}
			for (const auto& patternLine : pattern)
			{
				patternOutFile << patternLine << "\n";
			}
			patternOutFile << "\n";
			for (const auto& rotationLine : rotationPattern)
			{
				patternOutFile << rotationLine << "\n";
			}
			patternOutFile << "\n";
			for (auto& [instName, labelName] : instNameMapLabelName)
			{
				patternOutFile << instName << ", " << labelName << "\n";
			}
			patternOutFile << "\n";
			patternOutFile.close();
		}
	}
}

void Output::WriteCoBetterSolutionToFile(int topN, std::string fileName)
{
	fileName += "All_Best_Solutions";
	MKDIR(fileName.c_str());
	int index = 1;
	for (auto& tableScore : coTableScoreList)
	{
		if (index > topN)
		{
			break;
		}
		string solutionFileName = fileName + "/Best_Solution_" + to_string(index) + ".txt";
		ofstream outFile(solutionFileName);
		if (!outFile.is_open())
		{
			cerr << "Error opening file: " << solutionFileName << endl;
			return;
		}
		outFile << "group size: " << tableScore.first.GetGroupSize() << ", e.g. [" << tableScore.first.GetGroup(0, 0).GetSymbolNameSequence() << "]" << endl;
		outFile << "Best window size: " << tableScore.second.first << " x " << tableScore.second.second << "\n";
		auto tableStrings = tableScore.first.GetTableStringFormat();
		auto rotationStrings = tableScore.first.GetTableRotationFormat(leftS);
		
		int rowSpace = tableScore.second.second * tableScore.first.GetGroupSize();
		int colSpace = tableScore.second.first;
		for (const auto& rowString : tableStrings)
		{
			for (size_t j = 0; j < rowString.size(); ++j)
			{
				outFile << rowString[j];
				if ((j + 1) % rowSpace == 0)
				{
					outFile << " "; // add space after each window
				}
			}
			if ((&rowString - &tableStrings[0] + 1) % colSpace == 0)
			{
				outFile << "\n"; // add extra space after each column of windows
			}
			outFile << "\n";
		}
		
		outFile << "\n";
		
		outFile.close();
		
		
		index++;
	}
}