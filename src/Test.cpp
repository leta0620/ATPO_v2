#include "Test.h"

Test::Test()
{
	// Add your test functions here, and call them in the constructor
	this->TestInitialPlacement(8, 8)
	;
}

void Test::TestInitialPlacement(int groupSize, int rowNum)
{
	IntermidiateParser parser("test.txt");
	if (!parser.Parse()) {
		std::cerr << "Error: Failed to parse intermediate code file." << std::endl;
		return;
	}
	if (!parser.GenerateNetlistLookupTable()) {
		std::cerr << "Error: Failed to generate netlist lookup table." << std::endl;
		return;
	}

	for (const auto& cellName : parser.GetCommonSourceCellList()) {
		std::cout << "Common Source Cell: " << cellName << std::endl;
	}

	for (const auto& symbol : parser.GetNetlistLookupTable().GetAllSymbolNames()) {
		std::cout << "Symbol Name: " << symbol << std::endl;
	}
	for (const auto& symbol : parser.GetNetlistLookupTable().GetAllSymbolNames()) {
		const auto& netlistUnit = parser.GetNetlistLookupTable().GetNetlistUnit(symbol);
		std::cout << "Symbol: " << symbol
			<< ", Cell Name: " << netlistUnit.GetCellName()
			<< ", Analog Type: " << netlistUnit.GetAnalogType()
			<< ", Device Unit Count: " << netlistUnit.GetDeviceUnitCount()
			<< std::endl;
	}
	InitialPlacement initialPlacement(groupSize, rowNum, parser.GetNetlistLookupTable());
	std::vector<TableManager>& initialTableList = initialPlacement.GetInitialTableList();
	// Check if initialTableList is not empty
	if (initialTableList.empty()) {
		std::cerr << "Error: Initial table list is empty." << std::endl;
		return;
	}
	// Further checks can be added here to validate the contents of initialTableList
	for(auto& table : initialTableList)
	{
		std::cout << "Symbol :" << std::endl;
		for (int rowIdx = 0; rowIdx < table.GetRowSize(); ++rowIdx)
		{
			for (int colIdx = 0; colIdx < table.GetColSize(); ++colIdx)
			{
				const Group& group = table.GetGroup(rowIdx, colIdx);
				// 可以在這裡對 group 做進一步檢查或輸出
				for (auto& unit : group.GetDeviceUnits())
				{
					std::cout << unit.GetSymbol();
					// 可以在這裡對 device 做進一步檢查或輸出
				}
				std::cout << '|';
			}
			std::cout << std::endl;
		}

		std::cout << "Rotation :" << std::endl;

		for (int rowIdx = 0; rowIdx < table.GetRowSize(); ++rowIdx)
		{
			for (int colIdx = 0; colIdx < table.GetColSize(); ++colIdx)
			{
				const Group& group = table.GetGroup(rowIdx, colIdx);
				// 可以在這裡對 group 做進一步檢查或輸出
				for (auto& unit : group.GetDeviceUnits())
				{
					if (unit.GetRotation() == CellRotation::R0) 
					{
						std::cout << std::string("R0");
					}
					else if (unit.GetRotation() == CellRotation::MY)
					{
						std::cout << std::string("MY");
					}
					else
					{
						std::cout << std::string("d");
					}
					// 可以在這裡對 device 做進一步檢查或輸出
				}
				std::cout << '|';
			}
			std::cout << std::endl;
		}

		std::cout << std::endl;
		std::cout << std::endl;
	}
}

