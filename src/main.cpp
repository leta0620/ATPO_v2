#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include "IntermidiateParser.h"
#include "TableManager.h"
#include "InitialPlacement.h"
#include "SAManager.h"
#include "Test.h"
#include "Output.h"

using namespace std;

int main(int argc, char* argv[]) {
	// input commandLine arguments : groupSize, rowSize, itermidiate_code_file_path, output_file_path
	if (argc != 5) {
		cerr << "Usage: " << argv[0] << " <groupSize> <rowSize> <intermediate_code_file_path> <output_file_path>" << endl;
		return 1;
	}

	int groupSize = stoi(argv[1]);
	int row_num = stoi(argv[2]);
	string intermediate_code_file_path = argv[3];
	string output_file_path = argv[4];




	return 0;
}