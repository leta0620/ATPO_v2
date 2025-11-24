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
	if (argc != 6) {
		Test test; // Run tests
		cerr << "Usage: " << argv[0] << " <groupSize> <rowSize> <intermediate_code_file_path> <output_file_path> <thread_num>" << endl;
		return 1;
	}

	int groupSize = stoi(argv[1]);
	int row_num = stoi(argv[2]);
	string intermediate_code_file_path = argv[3];
	string output_file_path = argv[4];
	int thread_num = stoi(argv[5]);

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
			SAManager saManager(initialTableList[i], parser.GetNetlistLookupTable(), 0.95, 1000.0, 1.0, 100, true);
			allNondominatedSolutions[i] = saManager.GetNondominatedSolution();
		}

		Output output(groupSize, row_num, allNondominatedSolutions);

		output.WriteAllResultToFile(output_file_path);
		output.PrintAllResult();

		output.SelectSignificantNondominatedSolutions();
		output.WriteSignificantNondominatedSolutionsToFile("significant_" + output_file_path);
		output.PrintSignificantNondominatedSolutions();
	}
	else
	{
		// Multi-thread SA
		map<int, vector<TableManager>> allNondominatedSolutions;
		std::mutex resultsMutex;
		std::atomic<int> nextIndex{0};
		const int total = static_cast<int>(initialTableList.size());

		auto worker = [&]() {
			// 建立每個 thread 自己的隨機数產生器種子（如果 SAManager 內使用外部 rng，可視情況調整）
			std::random_device rd;
			auto tid_hash = std::hash<std::thread::id>{}(std::this_thread::get_id());
			std::mt19937 gen(static_cast<unsigned int>(rd() ^ tid_hash));

			while (true) {
				int i = nextIndex.fetch_add(1);
				if (i >= total) break;

				// 建立 SAManager（關閉命令列輸出以避免多執行緒輸出衝突）
				SAManager saManager(initialTableList[i], parser.GetNetlistLookupTable(), 0.95, 1000.0, 1.0, 100, false);
				// 若 SAManager 內部需要 rng 注入，需額外修改 SAManager；此處假設它內部自行處理。
				auto result = saManager.GetNondominatedSolution();

				{
					std::lock_guard<std::mutex> lock(resultsMutex);
					allNondominatedSolutions[i] = std::move(result);
				}
			}
		};

		// 啟動 thread_pool
		vector<std::thread> threads;
		threads.reserve(thread_num);
		for (int t = 0; t < thread_num; ++t) {
			threads.emplace_back(worker);
		}
		for (auto& th : threads) {
			if (th.joinable()) th.join();
		}

		// 使用 Output 處理結果（與單執行緒相同）
		Output output(groupSize, row_num, allNondominatedSolutions);

		output.WriteAllResultToFile(output_file_path);
		output.PrintAllResult();

		output.SelectSignificantNondominatedSolutions();
		output.WriteSignificantNondominatedSolutionsToFile("significant_" + output_file_path);
		output.PrintSignificantNondominatedSolutions();
	}

	return 0;
}