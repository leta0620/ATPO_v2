#include "InitialPlacement.h"
#include <algorithm>

InitialPlacement::InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist) : groupSize(groupSize), rowSize(rowSize), colSize(0), netListLookupTable(netlist)
{
	//InitialTableList.resize(rowSize, TableManager(groupSize, rowSize, colSize, netListLookupTable));
	this->CalculateColSize(); //floorplan
	this->InitialPathOrder();
	this->CalculateInitialTableList(); //placement
}

void InitialPlacement::GroupAllocation()
{
	// Implementation for group allocation

	// 由於要從common source作為起始點找出current mirror finger的連線關係而後創建group，而後將該group作為基數作為其他finger的模板
	// 因此要先找到group基數的模板，需要取最小數量的finger，其他finger則為該模板的倍數數量
	// 故先將common source代表元件依照數量由小到大排序
	std::vector<std::string> commonSourceOrder = netListLookupTable.GetCommonSourceList();
	std::stable_sort(commonSourceOrder.begin(), commonSourceOrder.end(),
		[this](const std::string& a, const std::string& b) -> bool
		{
			int countA = netListLookupTable.GetNetlistUnit(a).GetDeviceUnitCount();
			int countB = netListLookupTable.GetNetlistUnit(b).GetDeviceUnitCount();
			if (countA != countB)
				return countA < countB; // 升冪排序（數量小的在前）
			return a < b; // 次要鍵：字典序
		});
	// 先從基數最小做起
	// 先抓當前元件節點，然後依元件節點迭代，進而分配完一個group元件配置
	std::string nowSourceSymbol = commonSourceOrder[0];
	std::vector<int> deviceUnitCountList;
	while (nowSourceSymbol != "")
	{
		NetlistUnit unit = netListLookupTable.GetNetlistUnit(nowSourceSymbol);
		deviceUnitCountList.push_back(unit.GetDeviceUnitCount());
		nowSourceSymbol = netListLookupTable.GetPinDLinkWho(nowSourceSymbol).first;
	}
	int gcdValue = deviceUnitCountList[0];
	for (size_t i = 1; i < deviceUnitCountList.size(); ++i)
	{
		gcdValue = gcd(gcdValue, deviceUnitCountList[i]);
	}
	std::vector<int> commonDivisors;

	for (int i = 1; i <= std::sqrt(gcdValue); ++i)
	{
		if (gcdValue % i == 0)
		{
			commonDivisors.push_back(i);

			if (i != gcdValue / i)
				commonDivisors.push_back(gcdValue / i);
		}
	}
	std::sort(commonDivisors.begin(), commonDivisors.end());
	// commonDivisors 現在存有所有公因數，接下來依序嘗試這些公因數(可以將元件分成幾等分)，找到所有符合 group 的配置
	std::vector<std::vector<int>> validGroupConfigurations;
	std::vector<int> validGroupNumberConfigurations;
	for(auto& divisor : commonDivisors)
	{
		std::vector<int> currentGroupConfiguration;
		bool validConfiguration = true;
		for(auto& deviceUnitCount : deviceUnitCountList)
		{
			// 使用 groupCount 來建立 group 配置
			int groupUnitCount = deviceUnitCount / divisor;
			if (groupUnitCount % 2 != 0)
			{
				validConfiguration = false;
				break; // 若有任何一個元件的 groupCount 為奇數，則跳出迴圈嘗試下一個公因數
			}
			else
			{
				currentGroupConfiguration.push_back(groupUnitCount);
			}
		}
		if (validConfiguration)
		{
			validGroupConfigurations.push_back(currentGroupConfiguration);
			validGroupNumberConfigurations.push_back(divisor);
		}
	}

}

int gcd(int a, int b) {
	while (b != 0) {
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}

void InitialPlacement::InitialPathOrder()
{
	// Implementation for initial path order
	std::vector<std::string> commonSourceOrder = netListLookupTable.GetCommonSourceList();
	std::stable_sort(commonSourceOrder.begin(), commonSourceOrder.end(),
		[this](const std::string& a, const std::string& b) -> bool
		{
			int countA = netListLookupTable.GetNetlistUnit(a).GetDeviceUnitCount();
			int countB = netListLookupTable.GetNetlistUnit(b).GetDeviceUnitCount();
			if (countA != countB)
				return countA < countB; // 升冪排序（數量小的在前）
			return a < b; // 次要鍵：字典序
		});

	for (int i = 0; i < (int)commonSourceOrder.size(); ++i)
	{
		std::vector<DeviceUnit> currentPath;
		for (int j = 0; j < (int)commonSourceOrder.size(); ++j)
		{
			NetlistUnit unit = netListLookupTable.GetNetlistUnit(commonSourceOrder[j]);
			int unitCount = unit.GetDeviceUnitCount();
			while (unitCount > 0)
			{
				DeviceUnit deviceUnit;
				deviceUnit.SetSymbol(unit.GetSynbolName());
				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
				deviceUnit.SetRotation(CellRotation::MY);

				deviceUnit.SetWidth(unit.GetDeviceWidth());
				deviceUnit.SetInstName(unit.GetInstName());

				currentPath.push_back(deviceUnit);
				--unitCount;


				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).second != "")
				{
					NetlistUnit shareDeviceUnit = netListLookupTable.GetNetlistUnit(netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first);
					deviceUnit.SetSymbol(shareDeviceUnit.GetSynbolName());
					deviceUnit.SetAnalogCellType(shareDeviceUnit.GetAnalogType());
					deviceUnit.SetRotation(CellRotation::MY);

					deviceUnit.SetWidth(shareDeviceUnit.GetDeviceWidth());
					deviceUnit.SetInstName(shareDeviceUnit.GetInstName());

					currentPath.push_back(deviceUnit);

					deviceUnit.SetRotation(CellRotation::R0);
					currentPath.push_back(deviceUnit);
				}


				deviceUnit.SetSymbol(unit.GetSynbolName());
				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
				deviceUnit.SetRotation(CellRotation::R0);

				deviceUnit.SetWidth(unit.GetDeviceWidth());
				deviceUnit.SetInstName(unit.GetInstName());

				currentPath.push_back(deviceUnit);
				--unitCount;
			}
		}
		auto last = commonSourceOrder.back();
		commonSourceOrder.pop_back();
		commonSourceOrder.insert(commonSourceOrder.begin(), last);
		this->pathOrder.push_back(currentPath);
	}
}







// Calculate initial placement tables list
void InitialPlacement::CalculateInitialTableList()
{
	std::vector<std::vector<int>> rowOrderList;
	std::vector<int> rowOrder;
	if (rowSize > 0)
	{
		int mid = rowSize  / 2; // 若為偶數，取偏左的中心
		rowOrder.push_back(mid);
		for (int offset = 1; (int)rowOrder.size() < rowSize; ++offset)
		{
			int up = mid - offset;
			int down = mid + offset;
			if (up >= 0)
				rowOrder.push_back(up);
			if ((int)rowOrder.size() >= rowSize) break;
			if (down < rowSize)
				rowOrder.push_back(down);
		}
		rowOrderList.push_back(rowOrder);
		rowOrder.clear();
		for (int i = 0; i < rowSize; ++i)
		{
			rowOrder.push_back(i);
		}
		rowOrderList.push_back(rowOrder);
	}
	for (auto& currentRowOrder : rowOrderList)
	{
		for (auto& path : pathOrder)
		{
			TableManager tableManager(this->groupSize, this->rowSize, this->colSize, this->netListLookupTable);

			// 複製一份 path，使原始 pathOrder 不被破壞（必要時可改為移動語意）
			auto pathVec = path;

			// 反轉整個 pathVec，從尾排到頭
			std::reverse(pathVec.begin(), pathVec.end());
			int rowCount = 0;

			for (int currentRow : currentRowOrder)
			{
				for (int currentCol = 0; currentCol < this->colSize; ++currentCol)
				{
					std::vector<DeviceUnit> groupUnits;
					// 從尾端 pop out，最多加 `groupSize` 個 DeviceUnit（效能優於 erase(begin())）
					for (int i = 0; i < groupSize && !pathVec.empty(); ++i)
					{
						DeviceUnit currentUnit = pathVec.back();
						pathVec.pop_back();
						if (rowCount % 2 == 1)
						{
							currentUnit.FlipRotation();
							groupUnits.insert(groupUnits.begin(), currentUnit);
						}
						else
						{
							groupUnits.push_back(currentUnit);
						}
					}
					Group currentgroup;
					if(pathVec.empty())
					{
						for (int dummy = (int)groupUnits.size(); dummy < groupSize; ++dummy)
						{
							DeviceUnit dummyUnit;
							dummyUnit.SetSymbol("d");
							dummyUnit.SetAnalogCellType("d");
							dummyUnit.SetRotation(CellRotation::R0);
							groupUnits.push_back(dummyUnit);
						}
					}
					currentgroup.SetDeviceUnits(groupUnits);
					// 把 currentgroup 放入 tableManager（假設 API 為 PlaceGroup）
					
					if (rowCount % 2 == 1)
					{
						int assignCol = this->colSize - 1 - currentCol;
						tableManager.PlaceGroup(currentgroup, currentRow, assignCol);
					}
					else 
					{ 
						tableManager.PlaceGroup(currentgroup, currentRow, currentCol);
					}
				}
				rowCount++;
			}
			// 將 tableManager 加入 InitialTableList
			this->InitialTableList.push_back(std::move(tableManager));
		}
	}
}



void InitialPlacement::CalculateColSize()
{
	// Implementation for calculating column size
	int totalUnits = 0;
	int totalGroups = 0;
	for(auto& device : netListLookupTable.GetAllSymbolNames())
	{
		NetlistUnit unit = netListLookupTable.GetNetlistUnit(device);
		totalUnits += unit.GetDeviceUnitCount();
	}
	totalGroups = (totalUnits + groupSize - 1) / groupSize; // ceiling division
	this->colSize = (totalGroups + rowSize - 1) / rowSize; // ceiling division
	//std::cout << "Calculated column size: " << this->colSize << std::endl;
}