#pragma once
#include "TableManager.h"
#include "NetListLookupTable.h"
#include <random>
#include <string>

enum SAMode
{
	// random mode, CC mode
	RandomMode,
	CCMode
};

class SAManager
{
private:
	std::string circuitType;
	double coolRate;
	double initialTemp;
	double finalTemp;
	double currentTemp;
	int iterationPerTemp;
	bool openCommandLineOutput = false;

	std::vector<TableManager> nondominatedSolution;
	TableManager initialTable;
	TableManager nowUseTable;
	std::vector<TableManager> newTableList;

	NetlistLookupTable netlistLookupTable;

	SAMode saMode = SAMode::CCMode;


	// simulated annealing process
	void SAProcess();

	// generate new solution by perturbation
	void Perturbation(std::mt19937& gen);

	// select new usage table from current result(compare cost with mowUseTable and decide to accept or not)
	void SeleteNewUseTable(std::mt19937& gen);

	// compare newTableList with nondominatedSolution and update nondominatedSolution
	void UpdateNondominatedSolution();
public:
	SAManager(TableManager& initialTable, NetlistLookupTable& netlist, double coolRate = 0.95, double initialTemp = 1000.0, double finalTemp = 1.0, int iterationPerTemp = 100, bool openCommandLineOutput = false, std::string saMode = "RandomMode");

	std::vector<TableManager> GetNondominatedSolution() { return nondominatedSolution; }

};