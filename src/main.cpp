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
		// Multi-threaded SA (multi-start SA)
		const int jobCount = (int)initialTableList.size();
		if (jobCount == 0) {
			cerr << "Error: initialTableList is empty." << endl;
			return 1;
		}

		// 避免 thread_num > jobCount
		thread_num = min(thread_num, jobCount);
		if (thread_num <= 0) thread_num = 1;

		// ★ 建議把 lookup table 先存成 const ref，確保每個 thread 都只讀同一份資料
		auto& netlistLUT = parser.GetNetlistLookupTable();

		// 每個 job 的結果放在對應 index，最後再轉成 map
		vector<vector<TableManager>> results(jobCount);

		atomic<int> nextJob{ 0 };
		//mutex coutMutex; // 只用來鎖 cout，避免多執行緒輸出互相打架

		auto worker = [&]() {
			while (true) {
				int i = nextJob.fetch_add(1);
				if (i >= jobCount) break;

				//{
				//	lock_guard<mutex> lock(coutMutex);
				//	cout << "[Thread " << this_thread::get_id() << "] round: "
				//		<< (i + 1) << "/" << jobCount << endl;
				//}

				// ★ 重要：每個 SA run 用自己的 TableManager copy，避免任何共享可變狀態
				//TableManager initTable = initialTableList[i];

				// 跑 SA（你的參數照舊）
				SAManager saManager(initialTableList[i], netlistLUT, 0.9, 100.0, 1.0, 5, false);

				// 每個 index 只被寫一次 -> 不需要 mutex
				results[i] = saManager.GetNondominatedSolution();
			}
			};

		// 開 thread_num 個 worker threads
		vector<thread> threads;
		threads.reserve(thread_num);
		for (int t = 0; t < thread_num; ++t) {
			threads.emplace_back(worker);
		}
		for (auto& th : threads) th.join();

		// 組回你原本的 map<int, vector<TableManager>>
		map<int, vector<TableManager>> allNondominatedSolutions;
		for (int i = 0; i < jobCount; ++i) {
			allNondominatedSolutions[i] = std::move(results[i]);
		}

		// 後處理跟單執行緒版本一樣
		Output output(groupSize, row_num, allNondominatedSolutions, left_is_S_or_D,
			outerInput.GetLabelNameMapInstName(), outerInput.GetInstNameMapLabelName());

		output.WriteAllResultToFile(output_file_path + "output.txt");
		output.PrintAllResult();

		output.SelectSignificantNondominatedSolutions();
		output.WriteSignificantNondominatedSolutionsToFile(output_file_path + "output_significant.txt");
		output.PrintSignificantNondominatedSolutions();
	}

	return 0;
}