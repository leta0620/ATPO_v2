#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>

#include "IntermidiateParser.h"
#include "TableManager.h"
#include "InitialPlacement.h"
#include "SAManager.h"
#include "Test.h"
#include "Output.h"


using namespace std;

int main(int argc, char* argv[]) {
	// input commandLine arguments : groupSize, rowSize, itermidiate_code_file_path, output_file_path, thread_num
	if (argc == 2 && string(argv[1]) == "test") {
		// Run tests
		Test test;
		return 0;
	}

	//if (argc != 8) {
	//	Test test; // Run tests
	//	cerr << "Usage: " << argv[0] << " <groupSize> <rowSize> <CDL_input_file_path> <Pattern_input_file_path> <output_file_path> <thread_num> <left_is_S_or_D>" << endl;
	//	return 1;
	//}
	
	if (argc != 7) {
		Test test; // Run tests
		cerr << "Usage: " << argv[0] << " <groupSize> <rowSize> <CDL_input_file_path> <output_file_path> <thread_num> <left_is_S_or_D>" << endl;
		return 1;
	}



	int groupSize = stoi(argv[1]);
	int row_num = stoi(argv[2]);
	string cdl_input_file_path = argv[3];
	//string pattern_input_file_path = argv[4];
	string output_file_path = argv[4];
	int thread_num = stoi(argv[5]);
	string left_is_S_or_D = argv[6];

	// Generate intermediate file from CDL and Pattern files
	string intermediate_code_file_path = "intermediate_temp.txt";
	//OuterInput outerInput(cdl_input_file_path, pattern_input_file_path);
	OuterInput outerInput(cdl_input_file_path);
	outerInput.SetIntermidiateFile(intermediate_code_file_path);
	/*if (!outerInput.ParsePatternFile()) {
		cerr << "Error: Failed to parse pattern file." << endl;
		return 1;
	}*/
	if (!outerInput.ParseCdlFile()) {
		cerr << "Error: Failed to parse CDL file." << endl;
		return 1;
	}
	if (!outerInput.GenIntermidiateFile()) {
		cerr << "Error: Failed to generate intermediate file." << endl;
		return 1;
	}

	//IntermidiateParser parser(intermediate_code_file_path);
	IntermidiateParser parser(intermediate_code_file_path);
	if (!parser.Parse()) {
		cerr << "Error: Failed to parse intermediate code file." << endl;
		return 1;
	}
	
	if (!parser.GenerateNetlistLookupTable()) {
		cerr << "Error: Failed to generate netlist lookup table." << endl;
		return 1;
	}


	InitialPlacement initialPlacement(groupSize, row_num, parser.GetNetlistLookupTable());
	vector<TableManager>& initialTableList = initialPlacement.GetInitialTableList();

	// 
	if (thread_num == 1)
	{
		// Single thread SA
		map<int, vector<TableManager>> allNondominatedSolutions;
		for (int i = 0; i < initialTableList.size(); ++i)
		{
			cout << "round: " << i + 1 << "/" << initialTableList.size() << endl;
			SAManager saManager(initialTableList[i], parser.GetNetlistLookupTable(), 0.9, 100.0, 1.0, 5, true);
			allNondominatedSolutions[i] = saManager.GetNondominatedSolution();
			
			cout << "\r";
			cout << "                                        ";
			cout << "\r";
			cout << "\b";
		}
		cout << endl;

		Output output(groupSize, row_num, allNondominatedSolutions, left_is_S_or_D, outerInput.GetLabelNameMapInstName(), outerInput.GetInstNameMapLabelName());

		output.WriteAllResultToFile(output_file_path + "output.txt");
		output.PrintAllResult();

		output.SelectSignificantNondominatedSolutions();
		output.WriteSignificantNondominatedSolutionsToFile(output_file_path + "output_significant.txt");
		output.PrintSignificantNondominatedSolutions();
	}
	else
	{
		//

		cerr << "Error: Multi-threaded SA is not implemented in this version." << endl;
	}

	return 0;
}