#include "output.h"
#include <vector>
#include <map>
#include <string>
#include <fstream>
using namespace std; 


void Output::AddResultSingle(int round, TableManager& cTable)
{
	this->allNondominatedSolutions[round].push_back(cTable);
}

void Output::AddResultList(int round, std::vector<TableManager> cTableList)
{
	this->allNondominatedSolutions[round] = cTableList;
}

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
			//outFile << tableList[i].GetCostMap()[CostEnum::]
			for (auto cost : tableList[i].GetCostMap())
			{
				outFile << static_cast<int>(cost.first) << ":" << cost.second << "\t";
			}
			outFile << "\n";

			auto tableStrings = tableList[i].GetTableStringFormat();
			auto rotationStrings = tableList[i].GetTableRotationFormat();
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
			auto tableStrings = tableList[i].GetTableStringFormat();
			auto rotationStrings = tableList[i].GetTableRotationFormat();
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
}

void Output::PrintSignificantNondominatedSolutions()
{
	for (size_t i = 0; i < significantNondominatedSolutions.size(); ++i)
	{
		cout << "Significant Nondominated Solution " << i + 1 << ":\n";
		auto tableStrings = significantNondominatedSolutions[i].GetTableStringFormat();
		auto rotationStrings = significantNondominatedSolutions[i].GetTableRotationFormat();
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

void Output::WriteSignificantNondominatedSolutionsToFile(std::string fileName)
{
	ofstream outFile(fileName);
	if (!outFile.is_open())
	{
		cerr << "Error opening file: " << fileName << endl;
		return;
	}

	for (size_t i = 0; i < significantNondominatedSolutions.size(); ++i)
	{
		outFile << "Significant Nondominated Solution " << i + 1 << ":\n";
		auto tableStrings = significantNondominatedSolutions[i].GetTableStringFormat();
		auto rotationStrings = significantNondominatedSolutions[i].GetTableRotationFormat();
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
	outFile.close();
}