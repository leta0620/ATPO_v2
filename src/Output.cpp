#include "Output.h"
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <set>
#include <limits>
using namespace std; 


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
	for (int costEnumInt = 0; costEnumInt < 4; ++costEnumInt)
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
			auto costNameAndValue = sol.second[0].GetCostNameAndCostValueString();
			cout << "Cost Item : " << costNameAndValue[static_cast<int>(sol.first)].first << "\n";
		}

		for (size_t i = 0; i < sol.second.size(); ++i)
		{
			cout << "Significant Nondominated Solution " << i + 1 << ":\n";
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
			auto costNameAndValue = sol.second[0].GetCostNameAndCostValueString();
			outFile << "Cost Item : " << costNameAndValue[static_cast<int>(sol.first)].first << "\n";
			costItem = costNameAndValue[static_cast<int>(sol.first)].first;
		}

		for (size_t i = 0; i < sol.second.size(); ++i)
		{
			outFile << "Significant Nondominated Solution " << i + 1 << ":\n";
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

