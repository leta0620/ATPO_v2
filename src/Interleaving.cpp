#include "Interleaving.h"
#include <iostream>

using namespace std;

void Interleaving::Interleave(TableManager& tableManager, NetlistLookupTable& netlist)
{
	this->initialTable = tableManager;
	this->netlistLookupTable = netlist;
	// 計算成本並初始化 nondominatedSolution
	this->initialTable.CheckAndFixDummyWidth();
	this->initialTable.CalculateTableCost();
	this->nowUseTable = this->initialTable;
	this->nondominatedSolution.push_back(this->initialTable);
	// 開始 Interleaving 流程
	this->InterleaveProcess();
}

void Interleaving::InterleaveProcess()
{
	this->initialTable;
	cout << 0;
}