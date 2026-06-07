#include "InitialPlacement.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

InitialPlacement::InitialPlacement(int groupSize, int rowSize, NetlistLookupTable netlist, std::vector<CostEnum> costEnumList, bool busFlag, bool forceDirectAllocation) : groupSize(groupSize), rowSize(rowSize), colSize(0), netListLookupTable(netlist), costEnumList(costEnumList), busFlag(busFlag), forceDirectAllocation(forceDirectAllocation)
{
	//InitialTableList.resize(rowSize, TableManager(groupSize, rowSize, colSize, netListLookupTable));
	this->GroupAllocation(); //floorplan
	if (busFlag)
	{
		this->BusGroupAllocation();
	}
	this->CalculateInitialTableList(); //placement
	//this->oddGroupAllocation(); //floorplan
	//this->CalculateOddTableList(); //placement
}

int gcd(int a, int b) {
	while (b != 0) {
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}


namespace {
Group TakeNextGroupOrDummy(std::vector<Group>& groupsInCurrentTable, const Group& dummyGroup)
{
	if (!groupsInCurrentTable.empty()) {
		Group nowGroup = groupsInCurrentTable.back();
		groupsInCurrentTable.pop_back();
		return nowGroup;
	}
	return dummyGroup;
}

void PlaceNextGroup(TableManager& tableManager, std::vector<Group>& groupsInCurrentTable, const Group& dummyGroup, int placeRow, int placeCol, bool flip)
{
	Group nowGroup = TakeNextGroupOrDummy(groupsInCurrentTable, dummyGroup);
	if (flip) {
		nowGroup.FlipGroupRotation();
	}
	tableManager.PlaceGroup(nowGroup, placeRow, placeCol);
}

void PlaceGroupsFromCenter(TableManager& tableManager, std::vector<Group>& groupsInCurrentTable, const Group& dummyGroup, int rowSize, int colSize)
{
	for (int i = 0; i < colSize / 2; ++i)
	{
		const int rightCol = (colSize / 2) + i;
		const int leftCol = (colSize / 2) - 1 - i;

		if (rowSize % 2 == 1)
		{
			const int centerRow = rowSize / 2;
			PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, centerRow, rightCol, false);
			PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, centerRow, leftCol, true);

			for (int j = 1; j <= rowSize / 2; ++j)
			{
				const int lowerRow = centerRow + j;
				const int upperRow = centerRow - j;
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, lowerRow, rightCol, false);
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, upperRow, leftCol, true);
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, upperRow, rightCol, false);
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, lowerRow, leftCol, true);
			}
		}
		else
		{
			for (int j = 0; j < rowSize / 2; ++j)
			{
				const int lowerRow = (rowSize / 2) + j;
				const int upperRow = (rowSize / 2) - 1 - j;
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, lowerRow, rightCol, false);
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, upperRow, leftCol, true);
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, upperRow, rightCol, false);
				PlaceNextGroup(tableManager, groupsInCurrentTable, dummyGroup, lowerRow, leftCol, true);
			}
		}
	}
}
DeviceUnit BuildDeviceUnit(const NetlistUnit& unit, CellRotation rotation)
{
	DeviceUnit deviceUnit;
	deviceUnit.SetSymbol(unit.GetSynbolName());
	deviceUnit.SetAnalogCellType(unit.GetAnalogType());
	deviceUnit.SetWidth(unit.GetDeviceWidth());
	deviceUnit.SetInstName(unit.GetInstName());
	deviceUnit.SetRotation(rotation);
	return deviceUnit;
}

std::vector<std::string> BuildDrainBranch(NetlistLookupTable& netlistLookupTable, const std::string& startSymbol)
{
	std::vector<std::string> branch;
	std::unordered_set<std::string> visited;
	std::string currentSymbol = startSymbol;

	while (!currentSymbol.empty())
	{
		if (visited.find(currentSymbol) != visited.end())
		{
			break;
		}

		branch.push_back(currentSymbol);
		visited.insert(currentSymbol);

		auto next = netlistLookupTable.GetPinDLinkWho(currentSymbol);
		if (next.first.empty() || next.second.empty() || next.second == "D")
		{
			break;
		}

		currentSymbol = next.first;
	}

	return branch;
}

void AppendMirrorBranchPath(NetlistLookupTable& netlistLookupTable, const std::vector<std::string>& branch, std::vector<DeviceUnit>& currentPath)
{
	for (const std::string& symbol : branch)
	{
		NetlistUnit unit = netlistLookupTable.GetNetlistUnit(symbol);
		currentPath.push_back(BuildDeviceUnit(unit, CellRotation::MY));
	}

	for (auto it = branch.rbegin(); it != branch.rend(); ++it)
	{
		NetlistUnit unit = netlistLookupTable.GetNetlistUnit(*it);
		currentPath.push_back(BuildDeviceUnit(unit, CellRotation::R0));
	}
}
int CeilDiv(int numerator, int denominator)
{
	return (numerator + denominator - 1) / denominator;
}

int NextEven(int value)
{
	return (value % 2 == 0) ? value : value + 1;
}

std::vector<std::string> BuildMirroredSymbolList(const std::vector<std::string>& branch)
{
	std::vector<std::string> mirroredSymbols = branch;
	if (mirroredSymbols.size() >= 2)
	{
		for (int i = static_cast<int>(mirroredSymbols.size()) - 2; i >= 0; --i)
		{
			mirroredSymbols.push_back(mirroredSymbols[i]);
		}
	}
	return mirroredSymbols;
}

std::vector<int> BuildMirroredSlotConfiguration(const std::vector<int>& levelSlotSizes)
{
	std::vector<int> frontSlots;
	std::vector<int> backSlots;
	frontSlots.reserve(levelSlotSizes.size());
	backSlots.reserve(levelSlotSizes.size());

	for (size_t i = 0; i + 1 < levelSlotSizes.size(); ++i)
	{
		int slot = levelSlotSizes[i];
		int half = slot / 2;
		if (half % 2 != 0)
		{
			frontSlots.push_back(half);
			backSlots.push_back(half);
		}
		else
		{
			frontSlots.push_back(half + 1);
			backSlots.push_back(half - 1);
		}
	}

	std::vector<int> mirroredSlotConfiguration;
	mirroredSlotConfiguration.reserve(frontSlots.size() + 1 + backSlots.size());
	mirroredSlotConfiguration.insert(mirroredSlotConfiguration.end(), frontSlots.begin(), frontSlots.end());
	mirroredSlotConfiguration.push_back(levelSlotSizes.back());
	for (auto it = backSlots.rbegin(); it != backSlots.rend(); ++it)
	{
		mirroredSlotConfiguration.push_back(*it);
	}
	return mirroredSlotConfiguration;
}

Group BuildGroupFromSymbolSlots(
	NetlistLookupTable& netListLookupTable,
	const std::vector<std::string>& mirroredSymbols,
	const std::vector<int>& mirroredSlotConfiguration)
{
	std::vector<DeviceUnit> currentGroup;
	std::unordered_map<std::string, int> symbolCount;
	for (int i = 0; i < (int)mirroredSlotConfiguration.size(); ++i)
	{
		std::string currentNode = mirroredSymbols[i];
		int slotCount = mirroredSlotConfiguration[i];
		for (int count = 0; count < slotCount; ++count)
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

	Group group;
	group.SetDeviceUnits(currentGroup);
	return group;
}


std::string BuildDirectAllocationKey(const std::vector<int>& levelSlotSizes, const std::vector<int>& branchGroupCounts)
{
	std::string key;
	for (int slot : levelSlotSizes)
	{
		key += std::to_string(slot) + ",";
	}
	key += "|";
	for (int count : branchGroupCounts)
	{
		key += std::to_string(count) + ",";
	}
	return key;
}
void BuildDirectAllocationGroups(
	NetlistLookupTable& netListLookupTable,
	std::vector<std::vector<Group>>& allConfigurationGroupForTables,
	const std::vector<std::string>& commonSourceOrder)
{
	std::vector<std::vector<std::string>> branchSymbolsList;
	branchSymbolsList.reserve(commonSourceOrder.size());
	for (const std::string& commonSourceSymbol : commonSourceOrder)
	{
		branchSymbolsList.push_back(BuildDrainBranch(netListLookupTable, commonSourceSymbol));
	}
	if (branchSymbolsList.empty() || branchSymbolsList[0].empty())
	{
		return;
	}

	const int levelCount = (int)branchSymbolsList[0].size();
	for (const auto& branchSymbols : branchSymbolsList)
	{
		if ((int)branchSymbols.size() != levelCount)
		{
			return;
		}
	}

	int anchorLevel = 0;
	int minDeviceCount = netListLookupTable.GetNetlistUnit(branchSymbolsList[0][0]).GetDeviceUnitCount();
	for (int level = 0; level < levelCount; ++level)
	{
		for (const auto& branchSymbols : branchSymbolsList)
		{
			int deviceCount = netListLookupTable.GetNetlistUnit(branchSymbols[level]).GetDeviceUnitCount();
			if (deviceCount < minDeviceCount)
			{
				minDeviceCount = deviceCount;
				anchorLevel = level;
			}
		}
	}

	std::unordered_set<std::string> seenConfigurations;

	int maxAnchorCount = 0;
	for (const auto& branchSymbols : branchSymbolsList)
	{
		int anchorCount = netListLookupTable.GetNetlistUnit(branchSymbols[anchorLevel]).GetDeviceUnitCount();
		maxAnchorCount = std::max(maxAnchorCount, anchorCount);
	}

	for (int anchorSlotSize = NextEven(maxAnchorCount); anchorSlotSize >= 2; anchorSlotSize -= 2)
	{
		std::vector<int> branchGroupCounts;
		branchGroupCounts.reserve(branchSymbolsList.size());
		for (const auto& branchSymbols : branchSymbolsList)
		{
			int anchorCount = netListLookupTable.GetNetlistUnit(branchSymbols[anchorLevel]).GetDeviceUnitCount();
			branchGroupCounts.push_back(CeilDiv(anchorCount, anchorSlotSize));
		}

		std::vector<int> levelSlotSizes(levelCount, 0);
		for (int level = 0; level < levelCount; ++level)
		{
			for (int branchIndex = 0; branchIndex < (int)branchSymbolsList.size(); ++branchIndex)
			{
				int deviceCount = netListLookupTable.GetNetlistUnit(branchSymbolsList[branchIndex][level]).GetDeviceUnitCount();
				int requiredSlotSize = NextEven(CeilDiv(deviceCount, branchGroupCounts[branchIndex]));
				levelSlotSizes[level] = std::max(levelSlotSizes[level], requiredSlotSize);
			}
		}


		std::string key = BuildDirectAllocationKey(levelSlotSizes, branchGroupCounts);
		if (seenConfigurations.find(key) != seenConfigurations.end())
		{
			continue;
		}
		seenConfigurations.insert(key);

		std::vector<int> mirroredSlotConfiguration = BuildMirroredSlotConfiguration(levelSlotSizes);
		std::vector<Group> allGroupsInATable;
		for (int branchIndex = 0; branchIndex < (int)branchSymbolsList.size(); ++branchIndex)
		{
			std::vector<std::string> mirroredSymbols = BuildMirroredSymbolList(branchSymbolsList[branchIndex]);
			if (mirroredSymbols.size() != mirroredSlotConfiguration.size())
			{
				continue;
			}

			Group group = BuildGroupFromSymbolSlots(netListLookupTable, mirroredSymbols, mirroredSlotConfiguration);
			for (int count = 0; count < branchGroupCounts[branchIndex]; ++count)
			{
				allGroupsInATable.push_back(group);
			}
		}

		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = (int)allGroupsInATable.size();
		for (int i = 0; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();
			reverseAllGroupsInATable.push_back(nowGroup);
		}
		allConfigurationGroupForTables.push_back(reverseAllGroupsInATable);
	}
}
void BuildFactorAllocationConfigurations(
	const std::vector<int>& deviceUnitCountList,
	std::vector<std::vector<int>>& validGroupConfigurations,
	std::vector<int>& validGroupNumberConfigurations)
{
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

	for (auto& divisor : commonDivisors)
	{
		std::vector<int> currentGroupConfiguration;
		bool validConfiguration = true;
		for (auto& deviceUnitCount : deviceUnitCountList)
		{
			int groupUnitCount = deviceUnitCount / divisor;
			if (groupUnitCount % 2 != 0)
			{
				validConfiguration = false;
				break;
			}
			else
			{
				currentGroupConfiguration.push_back(groupUnitCount);
			}
		}
		if (validConfiguration)
		{
			int pivot = currentGroupConfiguration.back();

			std::vector<int> left;
			std::vector<int> rightSmall;
			left.reserve(currentGroupConfiguration.size() - 1);
			rightSmall.reserve(currentGroupConfiguration.size() - 1);

			for (size_t i = 0; i + 1 < currentGroupConfiguration.size(); ++i) {
				int n = currentGroupConfiguration[i];
				int half = n / 2;

				int big, small;
				if (half % 2 != 0) {
					big = half;
					small = half;
				}
				else {
					big = half + 1;
					small = half - 1;
				}

				left.push_back(big);
				rightSmall.push_back(small);
			}

			std::vector<int> finalGroupConfiguration;
			finalGroupConfiguration.reserve(left.size() + 1 + rightSmall.size());

			finalGroupConfiguration.insert(finalGroupConfiguration.end(), left.begin(), left.end());
			finalGroupConfiguration.push_back(pivot);

			for (auto it = rightSmall.rbegin(); it != rightSmall.rend(); ++it) {
				finalGroupConfiguration.push_back(*it);
			}
			validGroupConfigurations.push_back(finalGroupConfiguration);
			validGroupNumberConfigurations.push_back(divisor);
		}
	}
}

void BuildGroupsFromConfigurations(
	NetlistLookupTable& netListLookupTable,
	std::vector<std::vector<Group>>& allConfigurationGroupForTables,
	const std::vector<std::string>& commonSourceOrder,
	int commonSourceGcdValue,
	const std::vector<std::vector<int>>& validGroupConfigurations,
	const std::vector<int>& validGroupNumberConfigurations)
{
	for (int i = 0; i < (int)validGroupConfigurations.size(); ++i)
	{
		std::vector<Group> allGroupsInATable;
		for (int j = 0; j < commonSourceOrder.size(); ++j)
		{
			std::string nowNode = commonSourceOrder[j];
			std::vector<std::string> currentGroupSymbolList;
			int mutiNumber = netListLookupTable.GetNetlistUnit(nowNode).GetDeviceUnitCount() / commonSourceGcdValue;

			while (nowNode != "")
			{
				currentGroupSymbolList.push_back(nowNode);
				if (netListLookupTable.GetPinDLinkWho(commonSourceOrder[j]).first != nowNode && netListLookupTable.GetPinDLinkWho(nowNode).second != "D")
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
			for (int k = 0; k < mutiNumber * groupNumber; ++k)
			{
				allGroupsInATable.push_back(actCurrentGroup);
			}
		}

		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = allGroupsInATable.size();
		for (int i = 0; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();

			reverseAllGroupsInATable.push_back(nowGroup);
		}
		allConfigurationGroupForTables.push_back(reverseAllGroupsInATable);
	}
}
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
	int commonSourceGcdValue = netListLookupTable.GetNetlistUnit(commonSourceOrder[0]).GetDeviceUnitCount();
	for (size_t i = 1; i < commonSourceOrder.size(); ++i)
	{
		int commonSourceUnitCount = netListLookupTable.GetNetlistUnit(commonSourceOrder[i]).GetDeviceUnitCount();
		commonSourceGcdValue = gcd(commonSourceGcdValue, commonSourceUnitCount);
	}

	int minSourceUnitCount = netListLookupTable.GetNetlistUnit(commonSourceOrder[0]).GetDeviceUnitCount();
	int baseScale = minSourceUnitCount / commonSourceGcdValue;

	std::string nowSourceSymbol = commonSourceOrder[0];
	std::vector<int> deviceUnitCountList;
	while (nowSourceSymbol != "")
	{
		NetlistUnit unit = netListLookupTable.GetNetlistUnit(nowSourceSymbol);
		deviceUnitCountList.push_back(unit.GetDeviceUnitCount() / baseScale);
		if (netListLookupTable.GetPinDLinkWho(nowSourceSymbol).first != nowSourceSymbol && netListLookupTable.GetPinDLinkWho(nowSourceSymbol).second != "D")
			nowSourceSymbol = netListLookupTable.GetPinDLinkWho(nowSourceSymbol).first;
		else
			nowSourceSymbol = "";
	}

	std::vector<std::vector<int>> validGroupConfigurations;
	std::vector<int> validGroupNumberConfigurations;
	if (this->forceDirectAllocation)
	{
		BuildDirectAllocationGroups(this->netListLookupTable, this->allConfigurationGroupForTables, commonSourceOrder);
		return;
	}

	BuildFactorAllocationConfigurations(deviceUnitCountList, validGroupConfigurations, validGroupNumberConfigurations);
	if (validGroupConfigurations.empty())
	{
		BuildDirectAllocationGroups(this->netListLookupTable, this->allConfigurationGroupForTables, commonSourceOrder);
		return;
	}

	BuildGroupsFromConfigurations(
		this->netListLookupTable,
		this->allConfigurationGroupForTables,
		commonSourceOrder,
		commonSourceGcdValue,
		validGroupConfigurations,
		validGroupNumberConfigurations);


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
		
		int tableSize = int(groupsInATable.size() + 1) / 2;
		int nowTableColSize = ((tableSize + rowSize - 1) / rowSize) * 2;// ceiling division
		int nowTableGroupSize = groupsInATable[0].GetDeviceUnits().size();
		//std::cout << "Calculated column size: " << this->colSize << std::endl;
		TableManager nowTableManager(nowTableGroupSize, this->rowSize, nowTableColSize, this->netListLookupTable, this->costEnumList);

		Group dummyGroup; // dummy group for empty place
		/*for (int i = 0; i < nowTableGroupSize; ++i)
		{
			DeviceUnit dummyUnit;
			dummyUnit.SetSymbol("d");
			dummyUnit.SetAnalogCellType("DUMMY");
			dummyUnit.SetWidth(1);
			dummyUnit.SetInstName("d");
			dummyUnit.SetRotation(CellRotation::MY);
			dummyGroup.AddDeviceUnit(dummyUnit);
		}*/
		dummyGroup.BuildAllDummyGroup(nowTableGroupSize);

		std::vector<Group> groupsInCurrentTable = groupsInATable;
		// Place groups into the table
		PlaceGroupsFromCenter(nowTableManager, groupsInCurrentTable, dummyGroup, this->rowSize, nowTableColSize);
		this->InitialTableList.push_back(std::move(nowTableManager));
	}
}



void InitialPlacement::BusGroupAllocation()
{
	// Keep only the path that starts from the common-source device with the fewest units.
	std::vector<std::string> commonSourceOrder = netListLookupTable.GetCommonSourceList();
	std::stable_sort(commonSourceOrder.begin(), commonSourceOrder.end(),
		[this](const std::string& a, const std::string& b) -> bool
		{
			int countA = netListLookupTable.GetNetlistUnit(a).GetDeviceUnitCount();
			int countB = netListLookupTable.GetNetlistUnit(b).GetDeviceUnitCount();
			if (countA != countB)
				return countA < countB;
			return a < b;
		});

	std::vector<DeviceUnit> currentPath;
	for (const std::string& commonSourceSymbol : commonSourceOrder)
	{
		NetlistUnit commonSourceUnit = netListLookupTable.GetNetlistUnit(commonSourceSymbol);
		int unitCount = commonSourceUnit.GetDeviceUnitCount();
		std::vector<std::string> branch = BuildDrainBranch(netListLookupTable, commonSourceSymbol);

		while (unitCount > 0)
		{
			AppendMirrorBranchPath(netListLookupTable, branch, currentPath);
			unitCount -= 2;
		}
	}

	//this->pathOrder.clear();
	//this->pathOrder.push_back(currentPath);

	int commonBranchRepeatCount = 0;
	for (const std::string& commonSourceSymbol : commonSourceOrder)
	{
		int branchRepeatCount = netListLookupTable.GetNetlistUnit(commonSourceSymbol).GetDeviceUnitCount() / 2;
		commonBranchRepeatCount = (commonBranchRepeatCount == 0) ? branchRepeatCount : gcd(commonBranchRepeatCount, branchRepeatCount);
	}

	this->allConfigurationGroupForTables.clear();
	std::vector<std::string> minBranch = BuildDrainBranch(netListLookupTable, commonSourceOrder[0]);
	int branchPatternSize = (int)minBranch.size() * 2;
	for (int config = 1; config <= commonBranchRepeatCount; ++config)
	{
		if (commonBranchRepeatCount % config != 0)
		{
			continue;
		}

		int groupDeviceCount = branchPatternSize * config;
		if (groupDeviceCount <= 0 || currentPath.size() % groupDeviceCount != 0)
		{
			continue;
		}

		std::vector<Group> allGroupsInATable;
		for (int startIndex = 0; startIndex < (int)currentPath.size(); startIndex += groupDeviceCount)
		{
			std::vector<DeviceUnit> groupDeviceUnits(
				currentPath.begin() + startIndex,
				currentPath.begin() + startIndex + groupDeviceCount);

			Group group;
			group.SetDeviceUnits(groupDeviceUnits);
			allGroupsInATable.push_back(group);
		}

		std::vector<Group> reverseAllGroupsInATable;
		int totalGroups = (int)allGroupsInATable.size();
		for (int i = 0; i < totalGroups; ++i)
		{
			Group nowGroup = allGroupsInATable.back();
			allGroupsInATable.pop_back();
			reverseAllGroupsInATable.push_back(nowGroup);
		}
		this->allConfigurationGroupForTables.push_back(reverseAllGroupsInATable);
	}
}
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
		/*for (int i = 0; i < nowTableGroupSize; ++i)
		{
			DeviceUnit dummyUnit;
			dummyUnit.SetSymbol("d");
			dummyUnit.SetAnalogCellType("DUMMY");
			dummyUnit.SetWidth(1);
			dummyUnit.SetInstName("d");
			dummyUnit.SetRotation(CellRotation::MY);
			dummyGroup.AddDeviceUnit(dummyUnit);
		}*/
		dummyGroup.BuildAllDummyGroup(nowTableGroupSize);

		std::vector<Group> groupsInCurrentTable = groupsInATable;
		// Place groups into the table
		PlaceGroupsFromCenter(nowTableManager, groupsInCurrentTable, dummyGroup, this->rowSize, nowTableColSize);
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
		/*for (int i = 0; i < nowTableGroupSize; ++i)
		{
			DeviceUnit dummyUnit;
			dummyUnit.SetSymbol("d");
			dummyUnit.SetAnalogCellType("DUMMY");
			dummyUnit.SetWidth(1);
			dummyUnit.SetInstName("d");
			dummyUnit.SetRotation(CellRotation::MY);
			dummyGroup.AddDeviceUnit(dummyUnit);
		}*/
		dummyGroup.BuildAllDummyGroup(nowTableGroupSize);

		std::vector<Group> groupsInCurrentTable = groupsInATable;
		// Place groups into the table
		PlaceGroupsFromCenter(nowTableManager, groupsInCurrentTable, dummyGroup, this->rowSize, nowTableColSize);
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















