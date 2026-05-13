#include "CoMode.h"
#include "SAManager.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

using namespace std;

//------ Other function implementation ---------

//unordered_set<TableManager> GenAllRowSequenceTable(TableManager& initialTable, unordered_set<TableManager> tableList = {}, vector<int> sequence = {})
//{
//	if (sequence.size() != initialTable.GetRowSize())
//	{
//		for (int i = 0; i < initialTable.GetRowSize(); i++)
//		{
//			if (find(sequence.begin(), sequence.end(), i) == sequence.end())
//			{
//				vector<int> seq(sequence);
//				seq.push_back(i);
//				unordered_set<TableManager> tmp = GenAllRowSequenceTable(initialTable, tableList, seq);
//				tableList.insert(tmp.begin(), tmp.end());
//			}
//		}
//	}
//	else
//	{
//		
//		vector<vector<Group>> initialTableVV = initialTable.GetTable();
//		vector<vector<Group>> newTableVV(initialTable.GetRowSize(), vector<Group>(initialTable.GetColSize()));
//
//		for (int i = 0; i < sequence.size(); i++)
//		{
//			newTableVV[i] = initialTableVV[sequence[i]];
//		}
//
//		TableManager nTM(initialTable);
//		nTM.SetTable(newTableVV);
//
//		tableList.insert(nTM);
//	}
//
//	return tableList;
//}


//------ CMMode class function implementation ---------

void CoMode::BuildCoModeInitialTable()
{
	vector<int> deviceUnitNum = netlistLookupTable.GetAllDeviceUnitNums();
	
	// calculate all Common Divider in deviceUnitNum
	vector<int> commonOddDividers, commonEvenDividers;
	sort(deviceUnitNum.begin(), deviceUnitNum.end());
	int testCDTarget = deviceUnitNum[0];
	while (testCDTarget > 2)
	{
		bool isCommonDivider = true;
		for (int num : deviceUnitNum)
		{
			if (num % testCDTarget != 0)
			{
				isCommonDivider = false;
				break;
			}
		}

		if (isCommonDivider)
		{
			if (testCDTarget % 2 == 0)
			{
				commonEvenDividers.push_back(testCDTarget);
			}
			else
			{
				commonOddDividers.push_back(testCDTarget);
			}
		}

		testCDTarget--;
	}


	// build initial table based on common divider
	int branchNum = netlistLookupTable.GetCommonSourceList().size();
	if (netlistLookupTable.GetAllDeviceUnitNums().size() == netlistLookupTable.GetCommonSourceList().size())
	{
		// one device in each group

	}
	else if (netlistLookupTable.GetAllDeviceUnitNums().size() == netlistLookupTable.GetCommonSourceList().size() * 2)
	{
		// even divider
		for (int groupNum : commonEvenDividers)
		{
			vector<int> deviceNumList = netlistLookupTable.GetAllDeviceUnitNums();
			vector<int> groupNumList;
			int totalGroupNum = 0;
			for (int i = 0; i < deviceNumList.size(); i++)
			{
				groupNumList.push_back(deviceNumList[i] / groupNum);
				totalGroupNum += deviceNumList[i] / groupNum;
			}

			int colSize = ceil(static_cast<double>(totalGroupNum) / rowSize);
			int dummyGroupNum = rowSize * colSize - totalGroupNum;

			cout << "groupNum: " << groupNum << ", rowSize: " << rowSize << ", colSize: " << colSize << ", dummyGroupNum: " << dummyGroupNum << endl;
			for (int i = 0; i < groupNumList.size(); i++)
			{
				cout << groupNumList[i] << " ";
			}
			cout << endl;

			// even divider, even col size
			if (colSize % 2 == 0)
			{
				// build all group
				vector<string> symbolNames = netlistLookupTable.GetAllSymbolNames();
				unordered_map<string, Group> groupMap; // key: symbolName, value: Group
				for (const string& symbolName : symbolNames)
				{
					NetlistUnit unit = netlistLookupTable.GetNetlistUnit(symbolName);
					vector<string> commonSourceList = netlistLookupTable.GetCommonSourceList();

					Group group;
					bool isCommonSource = find(commonSourceList.begin(), commonSourceList.end(), symbolName) != commonSourceList.end();
					for (int i = 0; i < groupNum; i++)
					{
						DeviceUnit deviceUnit;
						deviceUnit.SetSymbol(symbolName);
						deviceUnit.SetAnalogCellType(unit.GetAnalogType());
						deviceUnit.SetWidth(unit.GetDeviceWidth());
						deviceUnit.SetInstName(unit.GetInstName());

						if (isCommonSource)
						{
							if (i % 2 == 0) deviceUnit.SetRotation(CellRotation::R0);
							else deviceUnit.SetRotation(CellRotation::MY);
						}
						else
						{
							if (i % 2 == 0) deviceUnit.SetRotation(CellRotation::MY);
							else deviceUnit.SetRotation(CellRotation::R0);
						}
						group.AddDeviceUnit(deviceUnit);
					}

					groupMap[symbolName] = group;
				}

				// Collect branch
				vector<vector<string>> branch;
				for (const string& commonSource : netlistLookupTable.GetCommonSourceList())
				{
					vector<string> currentBranch;
					currentBranch.push_back(commonSource);
					string currentNode = commonSource;
					while (netlistLookupTable.GetPinDLinkWho(currentNode).second != "")
					{
						currentNode = netlistLookupTable.GetPinDLinkWho(currentNode).first;
						currentBranch.push_back(currentNode);
					}
					branch.push_back(currentBranch);
				}


				// Symbol map to group num
				unordered_map<string, int> deviceSymbolMapGroupNumC; // key: symbolName, value: groupNum

				for (int i = 0; i < symbolNames.size(); i++)
				{
					deviceSymbolMapGroupNumC[symbolNames[i]] = groupNumList[i];
				}

				// build initial table
				vector<vector<Group>> initialTable;
				for (const vector<string>& b : branch)
				{
					if (b.size() != 2) // currently only support two groups, need to consider more groups in the future
					{
						cerr << "Error: Does not support more than two groups yet." << endl;
						return;
					}

					unordered_map<string, int> deviceSymbolMapGroupNum = deviceSymbolMapGroupNumC;
					while (deviceSymbolMapGroupNum[b[0]] > 0 || deviceSymbolMapGroupNum[b[1]] > 0)
					{
						vector<Group> currentBranchGroup;
						if (deviceSymbolMapGroupNum[b[0]] + deviceSymbolMapGroupNum[b[1]] < colSize)
						{
							// not enough group to fill the current column, need to place dummy group
							int needDummyNum = colSize - deviceSymbolMapGroupNum[b[0]] - deviceSymbolMapGroupNum[b[1]];

							int leftDummyNum = needDummyNum / 2;
							int rightDummy = needDummyNum - leftDummyNum;
							Group dummyGroup;
							dummyGroup.BuildAllDummyGroup(groupNum);
							// put left dummy
							for (int i = 0; i < leftDummyNum; i++)
							{
								currentBranchGroup.push_back(dummyGroup);
							}

							// put group
							if (leftDummyNum % 2 == 0)
							{
								while (deviceSymbolMapGroupNum[b[0]] > 0 || deviceSymbolMapGroupNum[b[1]] > 0)
								{
									if (deviceSymbolMapGroupNum[b[0]] > 0)
									{
										currentBranchGroup.push_back(groupMap[b[0]]);
										deviceSymbolMapGroupNum[b[0]]--;
									}
									if (deviceSymbolMapGroupNum[b[1]] > 0)
									{
										currentBranchGroup.push_back(groupMap[b[1]]);
										deviceSymbolMapGroupNum[b[1]]--;
									}
								}
							}
							else
							{
								while (deviceSymbolMapGroupNum[b[0]] > 0 || deviceSymbolMapGroupNum[b[1]] > 0)
								{
									if (deviceSymbolMapGroupNum[b[1]] > 0)
									{
										currentBranchGroup.push_back(groupMap[b[1]]);
										deviceSymbolMapGroupNum[b[1]]--;
									}
									if (deviceSymbolMapGroupNum[b[0]] > 0)
									{
										currentBranchGroup.push_back(groupMap[b[0]]);
										deviceSymbolMapGroupNum[b[0]]--;
									}
								}
							}
							//for (int i = 0; i < deviceSymbolMapGroupNum[b[0]]; i++)
							//{
							//	currentBranchGroup.push_back(groupMap[b[0]]);
							//	//deviceSymbolMapGroupNum[b[0]]--;
							//}
							//deviceSymbolMapGroupNum[b[0]] = 0;
							//for (int i = 0; i < deviceSymbolMapGroupNum[b[1]]; i++)
							//{
							//	currentBranchGroup.push_back(groupMap[b[1]]);
							//	//deviceSymbolMapGroupNum[b[1]]--;
							//}
							//deviceSymbolMapGroupNum[b[1]] = 0;

							// put right dummy
							for (int i = 0; i < rightDummy; i++)
							{
								currentBranchGroup.push_back(dummyGroup);
							}
						}
						else
						{
							for (int cols = 0; cols < colSize; cols++)
							{
								if (deviceSymbolMapGroupNum[b[0]] != 0 && deviceSymbolMapGroupNum[b[1]] != 0)
								{
									if (cols % 2 == 0)
									{
										currentBranchGroup.push_back(groupMap[b[0]]);
										deviceSymbolMapGroupNum[b[0]]--;
									}
									else
									{
										currentBranchGroup.push_back(groupMap[b[1]]);
										deviceSymbolMapGroupNum[b[1]]--;
									}
								}
								else if (deviceSymbolMapGroupNum[b[0]] != 0 && deviceSymbolMapGroupNum[b[1]] == 0)
								{
									currentBranchGroup.push_back(groupMap[b[0]]);
									deviceSymbolMapGroupNum[b[0]]--;
								}
								else if (deviceSymbolMapGroupNum[b[0]] == 0 && deviceSymbolMapGroupNum[b[1]] != 0)
								{
									currentBranchGroup.push_back(groupMap[b[1]]);
									deviceSymbolMapGroupNum[b[1]]--;
								}
								else
								{
									cerr << "Error: Both groups in the branch have been fully placed, but still have columns to fill." << endl;
								}

							}
						}

						initialTable.push_back(currentBranchGroup);
					}
						
				}

				// fix has dummy row to top and bottom
				vector<int> hasDummyRowIndex;
				vector<int> noDummyRowIndex;
				vector<vector<Group>> hasDummyRow;
				for (int r = 0; r < initialTable.size(); r++)
				{
					bool hasDummy = false;
					for (int i = 0; i < initialTable[r].size(); i++)
					{
						if (initialTable[r][i].HasDummyUnit())
						{
							hasDummyRowIndex.push_back(r);
							hasDummyRow.push_back(initialTable[r]);
							hasDummy = true;
							break;
						}
					}
					if (!hasDummy)
					{
						noDummyRowIndex.push_back(r);
					}
				}

				vector<vector<Group>> newInitialTable;
				int hasDummyRowNum = hasDummyRow.size();
				int topDummyRowNum = hasDummyRowNum / 2;
				int dummyRowIndex = 0;
				for (; dummyRowIndex < topDummyRowNum; dummyRowIndex++)
				{
					newInitialTable.push_back(hasDummyRow[dummyRowIndex]);
				}
				for (int i = 0; i < noDummyRowIndex.size(); i++)
				{
					newInitialTable.push_back(initialTable[noDummyRowIndex[i]]);
				}
				for (; dummyRowIndex < hasDummyRowNum; dummyRowIndex++)
				{
					newInitialTable.push_back(hasDummyRow[dummyRowIndex]);
				}
				initialTable = newInitialTable;

				// fix not enough row to fill the table
				if (initialTable.size() < this->rowSize)
				{
					// not enough row to fill the table, need to place dummy group in the remaining rows
					int dummyRowNumToPlace = this->rowSize - initialTable.size();
					Group dummyGroup;
					dummyGroup.BuildAllDummyGroup(groupNum);

					vector<Group> dummyRow(colSize, dummyGroup);

					// insert on top
					int topDummyRow = dummyRowNumToPlace / 2;
					for (int i = 0; i < topDummyRow; i++)
					{
						initialTable.insert(initialTable.begin(), dummyRow);
					}

					// insert on bottom
					int bottomDummyRow = dummyRowNumToPlace - topDummyRow;
					for (int i = 0; i < bottomDummyRow; i++)
					{
						initialTable.push_back(dummyRow);
					}
				}

				// build initial table manager
				TableManager iniTable(groupNum, this->rowSize, colSize, netlistLookupTable, this->costEnumList);
				iniTable.SetTable(initialTable);

				// run SA
				SAManager sa(iniTable, netlistLookupTable, 0.85, 100.0, 1.0, 5, true, "CoSpecialMode", this->costEnumList);
				vector<TableManager> saResult = sa.GetNondominatedSolution();
				tableList.insert(tableList.end(), saResult.begin(), saResult.end());

			}
			else
			{
				// odd col size
			}

		}

		//for (int groupNum : commonOddDividers)
		//{
		//	vector<int> deviceNumList = netlistLookupTable.GetAllDeviceUnitNums();
		//	vector<int> groupNumList;
		//	int totalGroupNum = 0;
		//	for (int i = 0; i < deviceNumList.size(); i++)
		//	{
		//		groupNumList.push_back(deviceNumList[i] / groupNum);
		//		totalGroupNum += deviceNumList[i] / groupNum;
		//	}

		//	int colSize = ceil(static_cast<double>(totalGroupNum) / rowSize);
		//	int dummyGroupNum = rowSize * colSize - totalGroupNum;
		//	//cout << "groupNum: " << groupNum << ", rowSize: " << rowSize << ", colSize: " << colSize << ", dummyGroupNum: " << dummyGroupNum << endl;
		//	//for (int i = 0; i < groupNumList.size(); i++)
		//	//{
		//	//	cout << groupNumList[i] << " ";
		//	//}
		//	//cout << endl;
		//}
	}
	else
	{
		// multiple devices in each group, need to consider common source and common divider
		cerr << "Error: Does not support multiple devices in each group yet." << endl;
	}


	return;
}

void CoMode::CalculateCoModeBestTable()
{

}

