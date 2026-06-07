#pragma once
#include <unordered_map>
#include <string>
#include <vector>

#include "NetlistUnit.h"

class NetlistLookupTable
{
public:
	std::pair<std::string, std::string> GetPinSLinkWho(std::string synbolName);
	std::pair<std::string, std::string> GetPinGLinkWho(std::string synbolName);
	std::pair<std::string, std::string> GetPinDLinkWho(std::string synbolName);

	void AddNetlistUnit(const NetlistUnit& netlistUnit);
	NetlistUnit GetNetlistUnit(const std::string& synbolName);

	void SetCommonSourceList(const std::vector<std::string>& sourceList) { this->commonSourceList = sourceList; }
	std::vector<std::string> GetCommonSourceList() const { return this->commonSourceList; }

	std::vector<std::string> GetAllSymbolNames();

	bool GetNoAllSourceCommonFlag() const { return noAllSourceCommonFlag; }
	void SetNoAllSourceCommonFlag(bool flag) { noAllSourceCommonFlag = flag; }
	void SetAllDeviceOnlyOneUnitFlag(bool flag) { allDeviceOnlyOneUnitFlag = flag; }
	bool GetAllDeviceOnlyOneUnitFlag() const { return allDeviceOnlyOneUnitFlag; }
private:
	std::vector<std::string> commonSourceList;

	std::unordered_map<std::string, NetlistUnit> netlistUnitMap; // key(synbolName) -> NetlistUnit

	bool noAllSourceCommonFlag = false;// true : common source沒有全部連在一起
	bool allDeviceOnlyOneUnitFlag = false; // true : all device only have one unit, so the group allocation can be simplified, and some cost can be simplified as well
};