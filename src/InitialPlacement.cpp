#include "InitialPlacement.h"
#include <algorithm>

InitialPlacement::InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist) : groupSize(groupSize), rowSize(rowSize), colSize(0), netListLookupTable(netlist)
{
	//InitialTableList.resize(rowSize, TableManager(groupSize, rowSize, colSize, netListLookupTable));
	this->CalculatecolSize(); //floorplan
	this->InitialPahtOrder();
	this->CalculateInitialTableList(); //placement
}

// Calculate initial placement tables list
void InitialPlacement::CalculateInitialTableList()
{
	for (auto& path : pathOrder)
	{
		TableManager tableManager(this->groupSize, this->rowSize, this->colSize, this->netListLookupTable);

		// 複製一份 path，使原始 pathOrder 不被破壞（必要時可改為移動語意）
		auto pathVec = path;
		bool rotateFlag = false;
		for (int currentRow = 0; currentRow < rowSize; ++currentRow) {
			for (int currentCol = 0; currentCol < colSize; ++currentCol) {
				std::vector<DeviceUnit> groupUnits;
				// 從前面開始 pop out，最多加 `groupSize` 個 DeviceUnit
				for (int i = 0; i < groupSize && !pathVec.empty(); ++i) {
					DeviceUnit currentUnit = pathVec.front();
					pathVec.erase(pathVec.begin());
					if (rotateFlag) {
						currentgroup.AddDeviceUnit();
					}
					else {
						pathVec.front().SetRotation(CellRotation::R0);
					}
					
					 // 從前面移除一個元素
				}
				// TODO: 把 currentgroup 放入 tableManager，例如：
				tableManager.PlaceGroup(currentgroup, currentRow, currentCol);
				// (實際方法名稱請依專案中 TableManager 定義調整)
			}
			rotateFlag = !rotateFlag;
		}

		// TODO: 若需要，將 tableManager 加入 InitialTableList，例如：
		// this->InitialTableList.push_back(std::move(tableManager));
	}
}

void InitialPlacement::InitialPahtOrder()
{
	// Implementation for initial path order
	std::vector<std::string> commonSourceOrder = netListLookupTable.GetCommonSourceList();
	std::stable_sort(commonSourceOrder.begin(), commonSourceOrder.end(),
		[this](const std::string& a, const std::string& b) -> bool {
			int countA = netListLookupTable.GetNetlistUnit(a).GetDeviceUnitCount();
			int countB = netListLookupTable.GetNetlistUnit(b).GetDeviceUnitCount();
			if (countA != countB)
				return countA < countB; // 升冪排序（數量小的在前）
			return a < b; // 次要鍵：字典序
		});
	std::vector<int> sourceUnitCounts;
	for (auto& decvice : commonSourceOrder)
	{
		sourceUnitCounts.push_back(netListLookupTable.GetNetlistUnit(decvice).GetDeviceUnitCount());
	}
	for (int i = 0; i < commonSourceOrder.size(); ++i) {
		std::vector<DeviceUnit> currentPath;
		for (int j = 0; j < commonSourceOrder.size(); ++j)
		{
			NetlistUnit unit = netListLookupTable.GetNetlistUnit(commonSourceOrder[j]);
			while (auto& _countof = sourceUnitCounts[j])
			{
				DeviceUnit deviceUnit;
				deviceUnit.SetSymbol(unit.GetSynbolName());
				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
				deviceUnit.SetRotation(CellRotation::MY);
				currentPath.push_back(deviceUnit);
				--sourceUnitCounts[j];
				NetlistUnit shareDeviceUnit = netListLookupTable.GetNetlistUnit(netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]));
				deviceUnit.SetSymbol(unit.GetSynbolName());
				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
				deviceUnit.SetRotation(CellRotation::MY);
				currentPath.push_back(deviceUnit);
				deviceUnit.SetRotation(CellRotation::R0);
				currentPath.push_back(deviceUnit);
				deviceUnit.SetSymbol(unit.GetSynbolName());
				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
				deviceUnit.SetRotation(CellRotation::R0);
				currentPath.push_back(deviceUnit);
				--sourceUnitCounts[j];
			}
		}
		auto last = commonSourceOrder.back();
		commonSourceOrder.pop_back();
		commonSourceOrder.insert(commonSourceOrder.begin(), last);
		this->pathOrder.push_back(currentPath);
	}
}

void InitialPlacement::CalculatecolSize()
{
	// Implementation for calculating column size
	int totalUnits = 0;
	for(auto& device : netListLookupTable.GetAllSymbolNames())
	{
		NetlistUnit unit = netListLookupTable.GetNetlistUnit(device);
		totalUnits += unit.GetDeviceUnitCount();
	}
	this->colSize = (totalUnits + groupSize - 1) / groupSize; // ceiling division
}