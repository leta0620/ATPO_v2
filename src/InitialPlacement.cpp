#include "InitialPlacement.h"
#include <algorithm>
#include <unordered_map>

InitialPlacement::InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist, std::vector<CostEnum> costEnumList) : groupSize(groupSize), rowSize(rowSize), colSize(0), netListLookupTable(netlist), costEnumList(costEnumList)
{
	//InitialTableList.resize(rowSize, TableManager(groupSize, rowSize, colSize, netListLookupTable));
	this->GroupAllocation(); //floorplan
	this->CalculateInitialTableList(); //placement
	this->oddGroupAllocation(); //floorplan
	this->CalculateOddTableList(); //placement
}

int gcd(int a, int b) {
	while (b != 0) {
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}


void InitialPlacement::regularGroupAllocation()
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
	std::vector<std::vector<int>> validDeviceIndexs;
	std::vector<int> validGroupNumberConfigurations;
	for (auto& divisor : commonDivisors)
	{
		if (divisor % 2 == 1) continue; // 避免數量分配為奇數
		std::vector<int> currentGroupConfiguration;
		bool validConfiguration = true;
		for (auto& deviceUnitCount : deviceUnitCountList)
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
			
			int pivot = currentGroupConfiguration.back();

			std::vector<int> frontLine;       
			std::vector<int> endLine; 
			std::vector<int> frontIndex;
			std::vector<int> endIndex;


			// 除了最後一個數字之外處理
			for (size_t i = 0; i + 1 < currentGroupConfiguration.size(); ++i) {
				int n = currentGroupConfiguration[i];
				int half = n / 2;

				if (half % 2 != 0) {      // half 是奇數 
					frontLine.push_back(half);	
					frontIndex.push_back(i);
				}
				else {                  // half 是偶數 
					frontLine.push_back(half - 1);
					frontIndex.push_back(i);
					endLine.push_back(1);
					endIndex.push_back(i);
				}
			}
			
			if ((!endLine.empty()) && (pivot / 2) % 2 != 0) { // 如果 endLine 不為空且 pivot 的一半是奇數
				continue; // 跳過這個配置，因為無法對稱分配
			}

			// 重新生成結果數列
			std::vector<int> finalGroupConfiguration;
			std::vector<int> finalDeviceIndex;



			finalGroupConfiguration.insert(finalGroupConfiguration.end(), frontLine.begin(), frontLine.end());
			finalDeviceIndex.insert(finalDeviceIndex.end(), frontIndex.begin(), frontIndex.end());


			finalGroupConfiguration.push_back(pivot/2);
			finalDeviceIndex.push_back(currentGroupConfiguration.size() - 1);

			// end那組反向塞回去
			for (auto it = endLine.rbegin(); it != endLine.rend(); ++it) {
				finalGroupConfiguration.push_back(*it);
			}
			for (auto it = endIndex.rbegin(); it != endIndex.rend(); ++it) {
				finalDeviceIndex.push_back(*it);
			}

			validGroupConfigurations.push_back(finalGroupConfiguration);
			validDeviceIndexs.push_back(finalDeviceIndex);
			validGroupNumberConfigurations.push_back(divisor);
		}
	}
	// 至此，validGroupConfigurations 已經存有所有符合 group 配置的可能性

	//for (auto& config : validGroupConfigurations)
	//{
	//	std::cout << "Valid Group Configuration: ";
	//	for (auto& num : config)
	//	{
	//		std::cout << num << " ";
	//	}
	//	std::cout << std::endl;
	//}
	//for (size_t i = 0; i < validGroupNumberConfigurations.size(); ++i)
	//{
	//	std::cout << "Valid Group Number Configuration: " << validGroupNumberConfigurations[i] << std::endl;
	//}


	for (int i = 0; i < (int)validGroupConfigurations.size(); ++i)
	{
		// 針對 common source 代表元件依序建立 group
		std::vector<Group> allGroupsInATable;
		//排列finger元件順序
		for (int j = 0; j < commonSourceOrder.size(); ++j)
		{

			std::string nowNode = commonSourceOrder[j];
			std::vector<std::string> currentGroupSymbolList;
			int mutiNumber = netListLookupTable.GetNetlistUnit(nowNode).GetDeviceUnitCount() / netListLookupTable.GetNetlistUnit(commonSourceOrder[0]).GetDeviceUnitCount();



			while (nowNode != "")
			{
				currentGroupSymbolList.push_back(nowNode);
				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first != nowNode)
					nowNode = netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first;
				else
					nowNode = "";
			}



			std::vector<DeviceUnit> currentGroup;
			std::vector<int> currentGroupConfiguration = validGroupConfigurations[i];
			std::vector<int> currentDeviceIndex = validDeviceIndexs[i];
			std::unordered_map<std::string, int> symbolCount;
			for (int k = 0; k < (int)currentGroupConfiguration.size(); ++k)
			{
				std::string currentNode = currentGroupSymbolList[currentDeviceIndex[k]];
				int idxCount = currentGroupConfiguration[k];
				for (int count = 0; count < idxCount; ++count)
				{
					symbolCount[currentNode]++;
					NetlistUnit unit = netListLookupTable.GetNetlistUnit(currentNode);
					DeviceUnit deviceUnit;
					deviceUnit.SetSymbol(unit.GetSynbolName());
					deviceUnit.SetAnalogCellType(unit.GetAnalogType());
					deviceUnit.SetWidth(unit.GetDeviceWidth());
					deviceUnit.SetInstName(unit.GetInstName());
					if (symbolCount[currentNode] % 2 == 1)
					{
						deviceUnit.SetRotation(CellRotation::MY);
					}
					else
					{
						deviceUnit.SetRotation(CellRotation::R0);
					}
					currentGroup.push_back(deviceUnit);
				}
			}
			Group actCurrentGroup;
			actCurrentGroup.SetDeviceUnits(currentGroup);
			Group mirrorGroup = actCurrentGroup;
			mirrorGroup.FlipGroupRotation();
			currentGroup.insert(currentGroup.end(), mirrorGroup.GetDeviceUnits().begin(), mirrorGroup.GetDeviceUnits().end());
			Group finalGroup;
			finalGroup.SetDeviceUnits(currentGroup);




			int groupNumber = validGroupNumberConfigurations[i];
			for (int k = 0; k < mutiNumber * groupNumber; ++k)
			{
				allGroupsInATable.push_back(finalGroup);
			}
		}

		// 反轉 group 順序
		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = allGroupsInATable.size();
		for (int i = 0; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();


			reverseAllGroupsInATable.push_back(nowGroup);
		}
		this->allConfigurationGroupForTables.push_back(reverseAllGroupsInATable);
	}

	//for (auto& groupsInATable : this->allConfigurationGroupForTables)
	//{
	//	std::cout << "Groups in a Table: " << std::endl;
	//	for (auto& group : groupsInATable)
	//	{
	//		for (auto& deviceUnit : group.GetDeviceUnits())
	//		{
	//			std::cout << deviceUnit.GetSymbol();
	//		}
	//		std::cout << std::endl;
	//	}
	//}
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
			int pivot = currentGroupConfiguration.back();

			std::vector<int> left;       // 鏡像點前：放大(或相同)的那個
			std::vector<int> rightSmall; // 收集小的那個，最後再反向當鏡像點後
			left.reserve(currentGroupConfiguration.size() - 1);
			rightSmall.reserve(currentGroupConfiguration.size() - 1);

			// 除了最後一個數字之外處理
			for (size_t i = 0; i + 1 < currentGroupConfiguration.size(); ++i) {
				int n = currentGroupConfiguration[i];
				int half = n / 2;

				int big, small;
				if (half % 2 != 0) {      // half 是奇數 -> (half, half)
					big = half;
					small = half;
				}
				else {                  // half 是偶數 -> (half+1, half-1)
					big = half + 1;
					small = half - 1;
				}

				left.push_back(big);
				rightSmall.push_back(small);
			}

			// 重新生成結果數列
			std::vector<int> finalGroupConfiguration;
			finalGroupConfiguration.reserve(left.size() + 1 + rightSmall.size());

			// 鏡像點前
			finalGroupConfiguration.insert(finalGroupConfiguration.end(), left.begin(), left.end());

			// 鏡像點
			finalGroupConfiguration.push_back(pivot);

			// 鏡像點後：小的那組反向塞回去
			for (auto it = rightSmall.rbegin(); it != rightSmall.rend(); ++it) {
				finalGroupConfiguration.push_back(*it);
			}
			validGroupConfigurations.push_back(finalGroupConfiguration);
			validGroupNumberConfigurations.push_back(divisor);
		}
	}
	// 至此，validGroupConfigurations 已經存有所有符合 group 配置的可能性

	//for (auto& config : validGroupConfigurations)
	//{
	//	std::cout << "Valid Group Configuration: ";
	//	for (auto& num : config)
	//	{
	//		std::cout << num << " ";
	//	}
	//	std::cout << std::endl;
	//}
	//for (size_t i = 0; i < validGroupNumberConfigurations.size(); ++i)
	//{
	//	std::cout << "Valid Group Number Configuration: " << validGroupNumberConfigurations[i] << std::endl;
	//}


	for (int i = 0; i < (int)validGroupConfigurations.size(); ++i)
	{
		// 針對 common source 代表元件依序建立 group
		std::vector<Group> allGroupsInATable;
		//排列finger元件順序
		for (int j = 0; j < commonSourceOrder.size(); ++j)
		{

			std::string nowNode = commonSourceOrder[j];
			std::vector<std::string> currentGroupSymbolList;
			int mutiNumber = netListLookupTable.GetNetlistUnit(nowNode).GetDeviceUnitCount() / netListLookupTable.GetNetlistUnit(commonSourceOrder[0]).GetDeviceUnitCount();
			


			while (nowNode != "")
			{
				currentGroupSymbolList.push_back(nowNode);
				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first != nowNode)
					nowNode = netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first;
				else 
					nowNode = "";
			}
			if (currentGroupSymbolList.size() >= 2)
			{
				for (int k = static_cast<int>(currentGroupSymbolList.size()) - 2; k >= 0; --k) {
					currentGroupSymbolList.push_back(currentGroupSymbolList[k]);
				}
			}



			std::vector<DeviceUnit> currentGroup;
			std::vector<int> currentGroupConfiguration = validGroupConfigurations[i];
			std::unordered_map<std::string, int> symbolCount;
			for (int k = 0; k < (int)currentGroupConfiguration.size(); ++k)
			{
				std::string currentNode = currentGroupSymbolList[k];
				int idxCount = currentGroupConfiguration[k];
				for (int count = 0; count < idxCount; ++count)
				{
					symbolCount[currentNode]++;
					NetlistUnit unit = netListLookupTable.GetNetlistUnit(currentNode);
					DeviceUnit deviceUnit;
					deviceUnit.SetSymbol(unit.GetSynbolName());
					deviceUnit.SetAnalogCellType(unit.GetAnalogType());
					deviceUnit.SetWidth(unit.GetDeviceWidth());
					deviceUnit.SetInstName(unit.GetInstName());
					if (symbolCount[currentNode] % 2 == 1)
					{
						deviceUnit.SetRotation(CellRotation::MY);
					}
					else
					{
						deviceUnit.SetRotation(CellRotation::R0);
					}
					currentGroup.push_back(deviceUnit);
				}
			}
			Group actCurrentGroup;
			actCurrentGroup.SetDeviceUnits(currentGroup);
			int groupNumber = validGroupNumberConfigurations[i];
			for (int k = 0; k < mutiNumber*groupNumber; ++k)
			{
				allGroupsInATable.push_back(actCurrentGroup);
			}
		}

		// 反轉 group 順序
		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = allGroupsInATable.size();
		for (int i = 0 ; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();


			reverseAllGroupsInATable.push_back(nowGroup);
		}
		this->allConfigurationGroupForTables.push_back(reverseAllGroupsInATable);
	}

	//for (auto& groupsInATable : this->allConfigurationGroupForTables)
	//{
	//	std::cout << "Groups in a Table: " << std::endl;
	//	for (auto& group : groupsInATable)
	//	{
	//		for (auto& deviceUnit : group.GetDeviceUnits())
	//		{
	//			std::cout << deviceUnit.GetSymbol();
	//		}
	//		std::cout << std::endl;
	//	}
	//}
}



// Calculate initial placement tables list
void InitialPlacement::CalculateInitialTableList()
{
	for (auto& groupsInATable : this->allConfigurationGroupForTables)
	{
		
		int tableSize = groupsInATable.size() / 2;
		int nowTableColSize = ((tableSize + rowSize - 1) / rowSize) * 2;// ceiling division
		int nowTableGroupSize = groupsInATable[0].GetDeviceUnits().size();
		//std::cout << "Calculated column size: " << this->colSize << std::endl;
		TableManager nowTableManager(nowTableGroupSize, this->rowSize, nowTableColSize, this->netListLookupTable, this->costEnumList);

		Group dummyGroup; // dummy group for empty place
		for (int i = 0; i < nowTableGroupSize; ++i)
		{
			DeviceUnit dummyUnit;
			dummyUnit.SetSymbol("d");
			dummyUnit.SetAnalogCellType("DUMMY");
			dummyUnit.SetWidth(1);
			dummyUnit.SetInstName("d");
			dummyUnit.SetRotation(CellRotation::MY);
			dummyGroup.AddDeviceUnit(dummyUnit);
		}

		std::vector<Group> groupsInCurrentTable = groupsInATable;
		// Place groups into the table
		for (int i = 0; i < nowTableColSize / 2; ++i)
		{
			for (int j = 0; j < this->rowSize / 2; ++j)
			{
				int placeRow;
				int placeCol;
				Group nowGroup;
				if (rowSize % 2 == 1)
				{
					if (j == 0) 
					{
						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeCol = (nowTableColSize / 2) - 1 - i ;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
					}
					else
					{
						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) - j;
						placeCol = (nowTableColSize / 2) - 1 - i ;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) - j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
					}
				}
				else
				{
					placeRow = (this->rowSize / 2) + j;
					placeCol = (nowTableColSize / 2) + i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) - 1 - j;
					placeCol = (nowTableColSize / 2) - 1 - i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowGroup.FlipGroupRotation();
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) - 1 - j;
					placeCol = (nowTableColSize / 2) + i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) + j;
					placeCol = (nowTableColSize / 2) - 1 - i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowGroup.FlipGroupRotation();
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
				}
			}
		}
		this->InitialTableList.push_back(std::move(nowTableManager));
	}
}



//void InitialPlacement::InitialPathOrder()
//{
//	// Implementation for initial path order
//	std::vector<std::string> commonSourceOrder = netListLookupTable.GetCommonSourceList();
//	std::stable_sort(commonSourceOrder.begin(), commonSourceOrder.end(),
//		[this](const std::string& a, const std::string& b) -> bool
//		{
//			int countA = netListLookupTable.GetNetlistUnit(a).GetDeviceUnitCount();
//			int countB = netListLookupTable.GetNetlistUnit(b).GetDeviceUnitCount();
//			if (countA != countB)
//				return countA < countB; // 升冪排序（數量小的在前）
//			return a < b; // 次要鍵：字典序
//		});
//
//	for (int i = 0; i < (int)commonSourceOrder.size(); ++i)
//	{
//		std::vector<DeviceUnit> currentPath;
//		for (int j = 0; j < (int)commonSourceOrder.size(); ++j)
//		{
//			NetlistUnit unit = netListLookupTable.GetNetlistUnit(commonSourceOrder[j]);
//			int unitCount = unit.GetDeviceUnitCount();
//			while (unitCount > 0)
//			{
//				DeviceUnit deviceUnit;
//				deviceUnit.SetSymbol(unit.GetSynbolName());
//				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
//				deviceUnit.SetRotation(CellRotation::MY);
//
//
//
//				currentPath.push_back(deviceUnit);
//				--unitCount;
//
//
//				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).second != "")
//				{
//					NetlistUnit shareDeviceUnit = netListLookupTable.GetNetlistUnit(netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first);
//					deviceUnit.SetSymbol(shareDeviceUnit.GetSynbolName());
//					deviceUnit.SetAnalogCellType(shareDeviceUnit.GetAnalogType());
//					deviceUnit.SetRotation(CellRotation::MY);
//
//					deviceUnit.SetWidth(shareDeviceUnit.GetDeviceWidth());
//					deviceUnit.SetInstName(shareDeviceUnit.GetInstName());
//
//					currentPath.push_back(deviceUnit);
//
//					deviceUnit.SetRotation(CellRotation::R0);
//					currentPath.push_back(deviceUnit);
//				}
//
//
//				deviceUnit.SetSymbol(unit.GetSynbolName());
//				deviceUnit.SetAnalogCellType(unit.GetAnalogType());
//				deviceUnit.SetRotation(CellRotation::R0);
//
//				deviceUnit.SetWidth(unit.GetDeviceWidth());
//				deviceUnit.SetInstName(unit.GetInstName());
//
//				currentPath.push_back(deviceUnit);
//				--unitCount;
//			}
//		}
//		auto last = commonSourceOrder.back();
//		commonSourceOrder.pop_back();
//		commonSourceOrder.insert(commonSourceOrder.begin(), last);
//		this->pathOrder.push_back(currentPath);
//	}
//}
//
//
//
//
//void InitialPlacement::CalculateColSize()
//{
//	// Implementation for calculating column size
//	int totalUnits = 0;
//	int totalGroups = 0;
//	for(auto& device : netListLookupTable.GetAllSymbolNames())
//	{
//		NetlistUnit unit = netListLookupTable.GetNetlistUnit(device);
//		totalUnits += unit.GetDeviceUnitCount();
//	}
//	totalGroups = (totalUnits + groupSize - 1) / groupSize; // ceiling division
//	this->colSize = (totalGroups + rowSize - 1) / rowSize; // ceiling division
//	//std::cout << "Calculated column size: " << this->colSize << std::endl;
//}

void InitialPlacement::oddGroupAllocation()
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
	std::vector<std::vector<int>> validFirstGroupConfigurations;
	std::vector<int> validFirstGroupNumberConfigurations;
	std::vector<std::vector<int>> validSecondGroupConfigurations;
	std::vector<int> validSecondGroupNumberConfigurations;

	for (auto& divisor : commonDivisors)
	{

		std::vector<int> currentGroupFirstConfiguration;
		std::vector<int> currentGroupSecondConfiguration;
		bool validConfiguration = false;
		if (deviceUnitCountList.size() == 2)
		{
			int deviceUnitCountZero = deviceUnitCountList[0] / divisor;
			int deviceUnitCountOne = deviceUnitCountList[1] / divisor;

			if (deviceUnitCountZero % 2 == 1 && deviceUnitCountOne % 2 == 1)
			{
				if (deviceUnitCountZero != 1 && divisor % 2 == 0) //commonsource units的數量不能為1，確保每個group都可以接回去commonsource。此外，為了讓device unit count在都是奇數的狀況可以重新對稱分配，一個group的device A + 1，另一個group的device A - 1，因此當commonsource units的數量為奇數且可形成總組數為偶數時，才有可能讓group重新對稱分配
				{ 
					validConfiguration = true;
					currentGroupFirstConfiguration.push_back((deviceUnitCountZero + 1));
					currentGroupSecondConfiguration.push_back((deviceUnitCountZero - 1));
					currentGroupFirstConfiguration.push_back((deviceUnitCountOne - 1));
					currentGroupSecondConfiguration.push_back((deviceUnitCountOne + 1));
				}
			}
		}

		if (validConfiguration)
		{
			int pivot = currentGroupFirstConfiguration.back();

			std::vector<int> left;       // 鏡像點前：放大(或相同)的那個
			std::vector<int> rightSmall; // 收集小的那個，最後再反向當鏡像點後
			left.reserve(currentGroupFirstConfiguration.size() - 1);
			rightSmall.reserve(currentGroupFirstConfiguration.size() - 1);

			// 除了最後一個數字之外處理
			for (size_t i = 0; i + 1 < currentGroupFirstConfiguration.size(); ++i) {
				int n = currentGroupFirstConfiguration[i];
				int half = n / 2;

				int big, small;
				if (half % 2 != 0) {      // half 是奇數 -> (half, half)
					big = half;
					small = half;
				}
				else {                  // half 是偶數 -> (half+1, half-1)
					big = half + 1;
					small = half - 1;
				}

				left.push_back(big);
				rightSmall.push_back(small);
			}

			// 重新生成結果數列
			std::vector<int> finalGroupConfiguration;
			finalGroupConfiguration.reserve(left.size() + 1 + rightSmall.size());

			// 鏡像點前
			finalGroupConfiguration.insert(finalGroupConfiguration.end(), left.begin(), left.end());

			// 鏡像點
			finalGroupConfiguration.push_back(pivot);

			// 鏡像點後：小的那組反向塞回去
			for (auto it = rightSmall.rbegin(); it != rightSmall.rend(); ++it) {
				finalGroupConfiguration.push_back(*it);
			}
			validFirstGroupConfigurations.push_back(finalGroupConfiguration);
			validFirstGroupNumberConfigurations.push_back((divisor / 2));
		}

		if (validConfiguration)
		{
			int pivot = currentGroupSecondConfiguration.back();

			std::vector<int> left;       // 鏡像點前：放大(或相同)的那個
			std::vector<int> rightSmall; // 收集小的那個，最後再反向當鏡像點後
			left.reserve(currentGroupSecondConfiguration.size() - 1);
			rightSmall.reserve(currentGroupSecondConfiguration.size() - 1);

			// 除了最後一個數字之外處理
			for (size_t i = 0; i + 1 < currentGroupSecondConfiguration.size(); ++i) {
				int n = currentGroupSecondConfiguration[i];
				int half = n / 2;

				int big, small;
				if (half % 2 != 0) {      // half 是奇數 -> (half, half)
					big = half;
					small = half;
				}
				else {                  // half 是偶數 -> (half+1, half-1)
					big = half + 1;
					small = half - 1;
				}

				left.push_back(big);
				rightSmall.push_back(small);
			}

			// 重新生成結果數列
			std::vector<int> finalGroupConfiguration;
			finalGroupConfiguration.reserve(left.size() + 1 + rightSmall.size());

			// 鏡像點前
			finalGroupConfiguration.insert(finalGroupConfiguration.end(), left.begin(), left.end());

			// 鏡像點
			finalGroupConfiguration.push_back(pivot);

			// 鏡像點後：小的那組反向塞回去
			for (auto it = rightSmall.rbegin(); it != rightSmall.rend(); ++it) {
				finalGroupConfiguration.push_back(*it);
			}
			validSecondGroupConfigurations.push_back(finalGroupConfiguration);
			validSecondGroupNumberConfigurations.push_back((divisor / 2));
		}
	}
	// 至此，validGroupConfigurations 已經存有所有符合 group 配置的可能性

	//for (auto& config : validGroupConfigurations)
	//{
	//	std::cout << "Valid Group Configuration: ";
	//	for (auto& num : config)
	//	{
	//		std::cout << num << " ";
	//	}
	//	std::cout << std::endl;
	//}
	//for (size_t i = 0; i < validGroupNumberConfigurations.size(); ++i)
	//{
	//	std::cout << "Valid Group Number Configuration: " << validGroupNumberConfigurations[i] << std::endl;
	//}


	for (int i = 0; i < (int)validFirstGroupConfigurations.size(); ++i)
	{
		// 針對 common source 代表元件依序建立 group
		std::vector<Group> allGroupsInATable;
		//排列finger元件順序
		for (int j = 0; j < commonSourceOrder.size(); ++j)
		{

			std::string nowNode = commonSourceOrder[j];
			std::vector<std::string> currentGroupSymbolList;
			int mutiNumber = netListLookupTable.GetNetlistUnit(nowNode).GetDeviceUnitCount() / netListLookupTable.GetNetlistUnit(commonSourceOrder[0]).GetDeviceUnitCount();



			while (nowNode != "")
			{
				currentGroupSymbolList.push_back(nowNode);
				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first != nowNode)
					nowNode = netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first;
				else
					nowNode = "";
			}
			if (currentGroupSymbolList.size() >= 2)
			{
				for (int k = static_cast<int>(currentGroupSymbolList.size()) - 2; k >= 0; --k) {
					currentGroupSymbolList.push_back(currentGroupSymbolList[k]);
				}
			}



			std::vector<DeviceUnit> currentGroup;
			std::vector<int> currentGroupConfiguration = validFirstGroupConfigurations[i];
			std::unordered_map<std::string, int> symbolCount;
			for (int k = 0; k < (int)currentGroupConfiguration.size(); ++k)
			{
				std::string currentNode = currentGroupSymbolList[k];
				int idxCount = currentGroupConfiguration[k];
				for (int count = 0; count < idxCount; ++count)
				{
					symbolCount[currentNode]++;
					NetlistUnit unit = netListLookupTable.GetNetlistUnit(currentNode);
					DeviceUnit deviceUnit;
					deviceUnit.SetSymbol(unit.GetSynbolName());
					deviceUnit.SetAnalogCellType(unit.GetAnalogType());
					deviceUnit.SetWidth(unit.GetDeviceWidth());
					deviceUnit.SetInstName(unit.GetInstName());
					if (symbolCount[currentNode] % 2 == 1)
					{
						deviceUnit.SetRotation(CellRotation::MY);
					}
					else
					{
						deviceUnit.SetRotation(CellRotation::R0);
					}
					currentGroup.push_back(deviceUnit);
				}
			}
			Group actCurrentGroup;
			actCurrentGroup.SetDeviceUnits(currentGroup);
			int groupNumber = validFirstGroupNumberConfigurations[i];
			for (int k = 0; k < mutiNumber * groupNumber; ++k)
			{
				allGroupsInATable.push_back(actCurrentGroup);
			}
		}

		// 反轉 group 順序
		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = allGroupsInATable.size();
		for (int i = 0; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();


			reverseAllGroupsInATable.push_back(nowGroup);
		}
		this->allOddFirstGroupForTables.push_back(reverseAllGroupsInATable);
	}


	for (int i = 0; i < (int)validSecondGroupConfigurations.size(); ++i)
	{
		// 針對 common source 代表元件依序建立 group
		std::vector<Group> allGroupsInATable;
		//排列finger元件順序
		for (int j = 0; j < commonSourceOrder.size(); ++j)
		{

			std::string nowNode = commonSourceOrder[j];
			std::vector<std::string> currentGroupSymbolList;
			int mutiNumber = netListLookupTable.GetNetlistUnit(nowNode).GetDeviceUnitCount() / netListLookupTable.GetNetlistUnit(commonSourceOrder[0]).GetDeviceUnitCount();



			while (nowNode != "")
			{
				currentGroupSymbolList.push_back(nowNode);
				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first != nowNode)
					nowNode = netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first;
				else
					nowNode = "";
			}
			if (currentGroupSymbolList.size() >= 2)
			{
				for (int k = static_cast<int>(currentGroupSymbolList.size()) - 2; k >= 0; --k) {
					currentGroupSymbolList.push_back(currentGroupSymbolList[k]);
				}
			}



			std::vector<DeviceUnit> currentGroup;
			std::vector<int> currentGroupConfiguration = validSecondGroupConfigurations[i];
			std::unordered_map<std::string, int> symbolCount;
			for (int k = 0; k < (int)currentGroupConfiguration.size(); ++k)
			{
				std::string currentNode = currentGroupSymbolList[k];
				int idxCount = currentGroupConfiguration[k];
				for (int count = 0; count < idxCount; ++count)
				{
					symbolCount[currentNode]++;
					NetlistUnit unit = netListLookupTable.GetNetlistUnit(currentNode);
					DeviceUnit deviceUnit;
					deviceUnit.SetSymbol(unit.GetSynbolName());
					deviceUnit.SetAnalogCellType(unit.GetAnalogType());
					deviceUnit.SetWidth(unit.GetDeviceWidth());
					deviceUnit.SetInstName(unit.GetInstName());
					if (symbolCount[currentNode] % 2 == 1)
					{
						deviceUnit.SetRotation(CellRotation::MY);
					}
					else
					{
						deviceUnit.SetRotation(CellRotation::R0);
					}
					currentGroup.push_back(deviceUnit);
				}
			}
			Group actCurrentGroup;
			actCurrentGroup.SetDeviceUnits(currentGroup);
			int groupNumber = validSecondGroupNumberConfigurations[i];
			for (int k = 0; k < mutiNumber * groupNumber; ++k)
			{
				allGroupsInATable.push_back(actCurrentGroup);
			}
		}

		// 反轉 group 順序
		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = allGroupsInATable.size();
		for (int i = 0; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();


			reverseAllGroupsInATable.push_back(nowGroup);
		}
		this->allOddSecondGroupForTables.push_back(reverseAllGroupsInATable);
	}

}

void InitialPlacement::CalculateOddTableList()
{
	std::vector<TableManager> oddFirstGroupTableList;
	for (auto& groupsInATable : this->allOddFirstGroupForTables)
	{
		int tableSize;
		if (groupsInATable.size() % 2 != 0)
		{
			tableSize = (groupsInATable.size() + 1) / 2;
		}
		else
		{
			tableSize = groupsInATable.size() / 2;
		}

		int nowTableColSize = ((tableSize + rowSize - 1) / rowSize) * 2;// ceiling division
		int nowTableGroupSize = groupsInATable[0].GetDeviceUnits().size();
		//std::cout << "Calculated column size: " << this->colSize << std::endl;
		TableManager nowTableManager(nowTableGroupSize, this->rowSize, nowTableColSize, this->netListLookupTable, this->costEnumList);

		Group dummyGroup; // dummy group for empty place
		for (int i = 0; i < nowTableGroupSize; ++i)
		{
			DeviceUnit dummyUnit;
			dummyUnit.SetSymbol("d");
			dummyUnit.SetAnalogCellType("DUMMY");
			dummyUnit.SetWidth(1);
			dummyUnit.SetInstName("d");
			dummyUnit.SetRotation(CellRotation::MY);
			dummyGroup.AddDeviceUnit(dummyUnit);
		}

		std::vector<Group> groupsInCurrentTable = groupsInATable;
		// Place groups into the table
		for (int i = 0; i < nowTableColSize / 2; ++i)
		{
			for (int j = 0; j < this->rowSize / 2; ++j)
			{
				int placeRow;
				int placeCol;
				Group nowGroup;
				if (rowSize % 2 == 1)
				{
					if (j == 0)
					{
						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
					}
					else
					{
						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) - j;
						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) - j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
					}
				}
				else
				{
					placeRow = (this->rowSize / 2) + j;
					placeCol = (nowTableColSize / 2) + i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) - 1 - j;
					placeCol = (nowTableColSize / 2) - 1 - i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowGroup.FlipGroupRotation();
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) - 1 - j;
					placeCol = (nowTableColSize / 2) + i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) + j;
					placeCol = (nowTableColSize / 2) - 1 - i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowGroup.FlipGroupRotation();
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
				}
			}
		}
		oddFirstGroupTableList.push_back(std::move(nowTableManager));
	}


	std::vector<TableManager> oddSecondGroupTableList;
	for (auto& groupsInATable : this->allOddSecondGroupForTables)
	{
		int tableSize;
		if (groupsInATable.size() % 2 != 0)
		{
			tableSize = (groupsInATable.size() + 1) / 2;
		}
		else
		{
			tableSize = groupsInATable.size() / 2;
		}

		int nowTableColSize = ((tableSize + rowSize - 1) / rowSize) * 2;// ceiling division
		int nowTableGroupSize = groupsInATable[0].GetDeviceUnits().size();
		//std::cout << "Calculated column size: " << this->colSize << std::endl;
		TableManager nowTableManager(nowTableGroupSize, this->rowSize, nowTableColSize, this->netListLookupTable, this->costEnumList);

		Group dummyGroup; // dummy group for empty place
		for (int i = 0; i < nowTableGroupSize; ++i)
		{
			DeviceUnit dummyUnit;
			dummyUnit.SetSymbol("d");
			dummyUnit.SetAnalogCellType("DUMMY");
			dummyUnit.SetWidth(1);
			dummyUnit.SetInstName("d");
			dummyUnit.SetRotation(CellRotation::MY);
			dummyGroup.AddDeviceUnit(dummyUnit);
		}

		std::vector<Group> groupsInCurrentTable = groupsInATable;
		// Place groups into the table
		for (int i = 0; i < nowTableColSize / 2; ++i)
		{
			for (int j = 0; j < this->rowSize / 2; ++j)
			{
				int placeRow;
				int placeCol;
				Group nowGroup;
				if (rowSize % 2 == 1)
				{
					if (j == 0)
					{
						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
					}
					else
					{
						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) - j;
						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) - j;
						placeCol = (nowTableColSize / 2) + i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

						placeRow = (this->rowSize / 2) + j;
						placeCol = (nowTableColSize / 2) - 1 - i;
						if (!groupsInCurrentTable.empty()) {
							nowGroup = groupsInCurrentTable.back();
							groupsInCurrentTable.pop_back();
						}
						else {
							nowGroup = dummyGroup;   // dummy group（用預設建構）
						}
						nowGroup.FlipGroupRotation();
						nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
					}
				}
				else
				{
					placeRow = (this->rowSize / 2) + j;
					placeCol = (nowTableColSize / 2) + i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) - 1 - j;
					placeCol = (nowTableColSize / 2) - 1 - i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowGroup.FlipGroupRotation();
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) - 1 - j;
					placeCol = (nowTableColSize / 2) + i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);

					placeRow = (this->rowSize / 2) + j;
					placeCol = (nowTableColSize / 2) - 1 - i;
					if (!groupsInCurrentTable.empty()) {
						nowGroup = groupsInCurrentTable.back();
						groupsInCurrentTable.pop_back();
					}
					else {
						nowGroup = dummyGroup;   // dummy group（用預設建構）
					}
					nowGroup.FlipGroupRotation();
					nowTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
				}
			}
		}
		oddSecondGroupTableList.push_back(std::move(nowTableManager));
	}
	for (int i = 0; i < (int)oddFirstGroupTableList.size(); ++i)
	{
		if (i >= (int)oddSecondGroupTableList.size())
		{
			break;
		}

		TableManager& firstTable = oddFirstGroupTableList[i];
		TableManager& secondTable = oddSecondGroupTableList[i];
		int mergedRowSize = firstTable.GetRowSize();
		int firstColSize = firstTable.GetColSize();
		int secondColSize = secondTable.GetColSize();
		int mergedColSize = firstColSize + secondColSize;
		int mergedGroupSize = firstTable.GetGroupSize();
		TableManager mergedTableManager(mergedGroupSize, mergedRowSize, mergedColSize, this->netListLookupTable, this->costEnumList);

		int firstHalfColSize = firstColSize / 2;
		int secondHalfColSize = secondColSize / 2;
		int mergedHalfColSize = mergedColSize / 2;

		std::vector<int> firstColumnMap(firstColSize, -1);
		if (firstColSize > 0)
		{
			firstColumnMap[0] = 0;
			firstColumnMap[firstColSize - 1] = mergedColSize - 1;
		}
		for (int col = 1; col < firstHalfColSize; ++col)
		{
			firstColumnMap[col] = mergedHalfColSize - (firstHalfColSize - 1) + (col - 1);
		}
		for (int col = firstHalfColSize; col < firstColSize - 1; ++col)
		{
			firstColumnMap[col] = mergedHalfColSize + (col - firstHalfColSize);
		}

		std::vector<int> secondColumnMap(secondColSize, -1);
		for (int col = 0; col < secondHalfColSize; ++col)
		{
			secondColumnMap[col] = 1 + col;
		}
		for (int col = secondHalfColSize; col < secondColSize; ++col)
		{
			secondColumnMap[col] = mergedHalfColSize + (firstHalfColSize - 1) + (col - secondHalfColSize);
		}

		for (int row = 0; row < mergedRowSize; ++row)
		{
			for (int col = 0; col < firstColSize; ++col)
			{
				int placeRow = row;
				int placeCol = firstColumnMap[col];
				if (placeCol >= 0 && placeCol < mergedColSize)
				{
					Group nowGroup = firstTable.GetGroup(row, col);
					mergedTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
				}
			}
			for (int col = 0; col < secondColSize; ++col)
			{
				int placeRow = row;
				int placeCol = secondColumnMap[col];
				if (placeCol >= 0 && placeCol < mergedColSize)
				{
					Group nowGroup = secondTable.GetGroup(row, col);
					mergedTableManager.PlaceGroup(nowGroup, placeRow, placeCol);
				}
			}
		}
		this->InitialTableList.push_back(std::move(mergedTableManager));

	}


}
