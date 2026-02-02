#include "SAManager.h"
#include "TableManager.h"
#include <vector>
#include <random>
#include <set>
#include <iostream>


using namespace std;

SAManager::SAManager(TableManager& initialTable, NetlistLookupTable& netlist, double coolRate, double initialTemp, double finalTemp, int iterationPerTemp, bool openCommandLineOutput, std::string saMode)
	: initialTable(initialTable)
	, netlistLookupTable(netlist)
	, coolRate(coolRate)
	, initialTemp(initialTemp)
	, finalTemp(finalTemp)
	, currentTemp(initialTemp)
	, iterationPerTemp(iterationPerTemp)
	, openCommandLineOutput(openCommandLineOutput)
{
	if (saMode == "RandomMode" || saMode == "0")
	{
		this->saMode = SAMode::RandomMode;
	}
	else if (saMode == "CCMode" || saMode == "1")
	{
		this->saMode = SAMode::CCMode;
	}
	else
	{
		cerr << "Unknown SA Mode, set to RandomMode by default." << endl;
		this->saMode = SAMode::RandomMode;
	}

	// 計算成本並初始化 nondominatedSolution
	this->initialTable.CheckAndFixDummyWidth();
	this->initialTable.CalculateTableCost();
	this->nowUseTable = this->initialTable;
	this->nondominatedSolution.push_back(this->initialTable);
	
	// 開始 SA 流程
	this->SAProcess();
}

void SAManager::SAProcess()
{
	// generate new solution
	std::random_device rd;
	std::mt19937 gen(rd());

	int nowIteration = 0;
	this->SetupGroupTypePositionMap();
	while (this->currentTemp > this->finalTemp)
	{
		newTableList.clear();

		this->Perturbation(gen);

		// calculate cost for new solution
		for (auto& r : this->newTableList)
		{
			r.CalculateTableCost();
		}

		// select new usage table
		this->SeleteNewUseTable(gen);

		// update nondominated solution
		this->UpdateNondominatedSolution();

		// cool down
		if (nowIteration >= this->iterationPerTemp)
		{
			this->currentTemp *= this->coolRate;
			nowIteration = 0;
			if (this->openCommandLineOutput)	cout << "\r" << "Current Temperature: " << this->currentTemp;
		}
		else
		{
			nowIteration++;
			//cout << "Current Iteration at this Temperature: " << nowIteration << endl;
		}
	}
}

void SAManager::Perturbation(std::mt19937& gen)
{
	auto SwapTwoGroupUnit = [](TableManager& table, mt19937& gen) {
		int rowSize = table.GetRowSize();
		int colSize = table.GetColSize();
		uniform_int_distribution<> disRow(0, rowSize - 1);
		uniform_int_distribution<> disCol(0, colSize - 1);
		int row1 = disRow(gen);
		int col1 = disCol(gen);
		int row2 = 0;
		int col2 = 0;
		do
		{
			row2 = disRow(gen);
			col2 = disCol(gen);
			Group group1 = table.GetGroup(row1, col1);
			Group group2 = table.GetGroup(row2, col2);

			int c = 0;
		} while (table.GetGroup(row1, col1).GetDeviceUnits() == table.GetGroup(row2, col2).GetDeviceUnits() || !table.CheckCanSwapGroups(row1, col1, row2, col2));

		if (!table.SwapGroups(row1, col1, row2, col2))
		{
			cerr << "Swap two group unit fail." << endl;
		}
	};

	auto SwapTwoCol = [](TableManager& table, mt19937& gen) {
		int rowSize = table.GetRowSize();
		int colSize = table.GetColSize();
		uniform_int_distribution<> disCol(0, colSize - 1);
		int col1 = disCol(gen);
		int col2 = 0;
		do
		{
			col2 = disCol(gen);
		} while (col1 == col2);
		// swap col1 and col2
		if (!table.SwapColumns(col1, col2))
		{
			cerr << "Swap two colume fail." << endl;
		}
	};

	auto SwapTwoRow = [](TableManager& table, mt19937& gen) {
		int rowSize = table.GetRowSize();
		int colSize = table.GetColSize();
		uniform_int_distribution<> disRow(0, rowSize - 1);
		int row1 = disRow(gen);
		int row2 = 0;
		do
		{
			row2 = disRow(gen);
		} while (row1 == row2);
		// swap row1 and row2
		if (!table.SwapRows(row1, row2))
		{
			cerr << "Swap two row fail." << endl;
		}
	};

	auto SwapCCTwoGroup = [](TableManager& table, mt19937& gen) {
		// TO DO: CC Mode Swap Two Group
		int rowS = table.GetRowSize();
		int colS = table.GetColSize();

		int leftS = colS / 2;
		
		// select one group from left side without dummy
		uniform_int_distribution<> disRowLeft(0, rowS - 1);
		uniform_int_distribution<> disColLeft(0, leftS - 1);
		int row1 = 0;
		int col1 = 0;
		do
		{
			row1 = disRowLeft(gen);
			col1 = disColLeft(gen);
		} while (table.GetGroup(row1, col1).HasDummyUnit());

		// select another one group from left side without dummy
		int row2 = 0;
		int col2 = 0;
		do
		{
			row2 = disRowLeft(gen);
			col2 = disColLeft(gen);
		} while ((row1 == row2 && col1 == col2) || table.GetGroup(row2, col2).HasDummyUnit());

		// swap the two group left side
		if (!table.SwapGroups(row1, col1, row2, col2))
		{
			cerr << "Swap two group unit fail." << endl;
		}

		int rightRow1 = rowS - 1 - row1;
		int rightCol1 = colS - 1 - col1;
		int rightRow2 = rowS - 1 - row2;
		int rightCol2 = colS - 1 - col2;

		// cout << "Swap CC Group: (" << row1 << ", " << col1 << ") with (" << row2 << ", " << col2 << ") and (" << rightRow1 << ", " << rightCol1 << ") with (" << rightRow2 << ", " << rightCol2 << ")" << endl;

		// swap the two group right side
		if (!table.SwapGroups(rightRow1, rightCol1, rightRow2, rightCol2))
		{
			cerr << "Swap two group unit fail." << endl;
		}
	};


	if (this->saMode == SAMode::RandomMode)
	{
		// generate new solutions by swapping two group unit
		TableManager newTable = this->nowUseTable;
		//newTable.PrintTableToConsole();

		if (this->currentTemp > (this->initialTemp + this->finalTemp) / 2)
		{
			uniform_int_distribution<> perOperation(0, 1);
			int op = perOperation(gen);
			if (op == 0)
				SwapTwoCol(newTable, gen);
			else
				SwapTwoRow(newTable, gen);
		}
		else
		{
			uniform_int_distribution<> perOperation(0, 2);
			int op = perOperation(gen);
			if (op == 0)
				SwapTwoCol(newTable, gen);
			else if (op == 1)
				SwapTwoRow(newTable, gen);
			else
				SwapTwoGroupUnit(newTable, gen);
		}

		this->newTableList.push_back(newTable);

	}
	else if (this->saMode == SAMode::CCMode)
	{
		// TO DO: CC Mode Perturbation
		TableManager newTable = this->nowUseTable;

		SwapCCTwoGroup(newTable, gen);
		this->newTableList.push_back(newTable);
	}
	else
	{
		cerr << "Unknown SA Mode." << endl;
	}
}

bool doesADominateB(const std::unordered_map<CostEnum, double>& aCost, const std::unordered_map<CostEnum, double>& bCost)
{
	bool aBetterInAtLeastOne = false;
	for (size_t i = 0; i < aCost.size(); ++i)
	{
		//if (aCost[i] > bCost[i])
		//{
		//	return false; // a is worse in this objective
		//}
		//else if (aCost[i] < bCost[i])
		//{
		//	aBetterInAtLeastOne = true; // a is better in this objective
		//}

		// 把上面改寫法成map 
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
}

void SAManager::SeleteNewUseTable(std::mt19937& gen)
{
	// test how many new solution can dominate NondominatedSolution Set
	double deltaBeDom = 0.0;	// the new solution is dominated by how many solutions in NondominatedSolution Set / total solutions in NondominatedSolution Set
	int dominateCount = 0;
	set<int> newBeOldDominatedIndex;	// index of solutions in NondominatedSolution Set which are dominated by new solution

	// if NondominatedSolution Set is empty, set deltaBeDom to 0
	if (this->nondominatedSolution.size() == 0)
	{
		deltaBeDom = 0.0;
		//cout << "NondominatedSolution Set is empty." << endl;
	}
	else {
		// compare each new solution with NondominatedSolution Set
		for (auto& nSol : this->nondominatedSolution)
		{
			for (int i = 0; i < this->newTableList.size(); i++)
			{
				if (doesADominateB(nSol.GetCostMap(), this->newTableList[i].GetCostMap()))
				{
					// new solution is dominated by nSet
					dominateCount++;
					newBeOldDominatedIndex.insert(i);
				}
			}
		}

		deltaBeDom = dominateCount / (this->nondominatedSolution.size() * this->newTableList.size());
	}

	// delete dominated solutions from newTableList
	vector<TableManager> updatedNewTableList;
	for (int i = 0; i < this->newTableList.size(); i++)
	{
		if (newBeOldDominatedIndex.find(i) == newBeOldDominatedIndex.end())
		{
			updatedNewTableList.push_back(this->newTableList[i]);
		}
	}
	this->newTableList = updatedNewTableList;
	if (newTableList.size() == 0)
	{
		//cout << "\t" << "no solution is better." << endl;
		return;
	}

	// check how many new solution dominate nowUseTable
	int newDominateNowUseCount = 0;
	vector<int> newDominateNowUseIndex;
	for (auto& newTable : this->newTableList)
	{
		if (doesADominateB(newTable.GetCostMap(), this->nowUseTable.GetCostMap()))
		{
			newDominateNowUseCount++;
			newDominateNowUseIndex.push_back(&newTable - &this->newTableList[0]);
		}
	}

	//cout << "\t" << newDominateNowUseCount << " new solutions dominate nowUseTable." << endl;





	// decide to accept new solution or not
	if (newDominateNowUseCount > 0)
	{
		//cout << "\t" << "Accept new solution which dominate nowUseTable." << endl;
		// random select one solution taht in newDominateNowUseIndex
		uniform_int_distribution<> dis(0, this->newTableList.size() - 1);
		int selectIndex = dis(gen);
		this->nowUseTable = this->newTableList[selectIndex];
	}
	else if (newDominateNowUseCount == 0 && newBeOldDominatedIndex.size() != this->newTableList.size())
	{
		// **no new solution dominate now solution**
		// accept with probability
		double acceptProb = 1 / (1 + exp((deltaBeDom) / this->currentTemp));
		uniform_real_distribution<> dis(0.0, 1.0);
		double randVal = dis(gen);
		if (randVal < acceptProb)
		{
			//cout << "\t" << "Accept new solution with probability." << endl;
			// find a new solution that not in oldDominateNewIndex
			uniform_int_distribution<> disNew(0, this->newTableList.size() - 1);
			int selectIndex = 0;
			this->nowUseTable = this->newTableList[selectIndex];
		}
		else
		{
			//cout << "\t" << "Reject new solution. probability." << endl;
		}
	}
	else
	{
		//cout << "\t" << "Reject new solution. No new solution better than now Solution." << endl;
	}
}

void SAManager::UpdateNondominatedSolution()
{
	set<int> toBeRemovedIndex;
	vector<int> newToBeAddIndex;
	for (auto& newTable : this->newTableList)
	{
		bool newIsDominated = false;
		std::unordered_map<CostEnum, double> newCost = newTable.GetCostMap();
		for (int i = 0; i < this->nondominatedSolution.size(); i++)
		{
			if (doesADominateB(newCost, this->nondominatedSolution[i].GetCostMap()))
			{
				toBeRemovedIndex.insert(i);
			}

			if (doesADominateB(this->nondominatedSolution[i].GetCostMap(), newCost))
			{
				newIsDominated = true;
			}
		}

		// if new solution is not dominated by any solution in nondominatedSolution, add it
		auto CheckNewTableExists = [&](TableManager& table) -> bool {
			for (const auto& existingTable : this->nondominatedSolution) {
				if (table.EqualTableToSelf(const_cast<TableManager&>(existingTable))) {
					return true; // Table already exists
				}
			}
			return false; // Table does not exist
			};

		// add new solution, only when it is not dominated and not already exists
		if (!newIsDominated && !CheckNewTableExists(newTable))
		{
			newToBeAddIndex.push_back(&newTable - &this->newTableList[0]);
		}
	}

	// remove dominated solutions from nondominatedSolution
	vector<TableManager> updatedNondominatedSolution;
	for (int i = 0; i < this->nondominatedSolution.size(); i++)
	{
		if (toBeRemovedIndex.find(i) == toBeRemovedIndex.end()) // not to be removed
		{
			updatedNondominatedSolution.push_back(this->nondominatedSolution[i]);
		}
	}
	// add new nondominated solutions
	for (auto index : newToBeAddIndex)
	{
		updatedNondominatedSolution.push_back(this->newTableList[index]);
	}
	this->nondominatedSolution = updatedNondominatedSolution;
}

void SAManager::SetupGroupTypePositionMap()
{
	this->groupTypePositionMap.clear();
	int rowSize = this->initialTable.GetRowSize();
	int colSize = this->initialTable.GetColSize();
	for (int r = 0; r < rowSize; r++)
	{
		for (int c = 0; c < colSize; c++)
		{
			Group group = this->initialTable.GetGroup(r, c);
			int typeHash = group.GetTypeHash();
			this->groupTypePositionMap[typeHash].push_back({ r, c });
		}
	}
}


