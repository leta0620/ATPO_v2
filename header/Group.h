#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <functional>

#include "DeviceUnit.h"

class Group
{
public:
	Group() = default;
	Group(int groupNum);

	// Adds a device unit to the group and recalculates the type hash
	void AddDeviceUnit(const DeviceUnit& deviceUnit);
	// Sets the device units of the group and recalculates the type hash
	void SetDeviceUnits(const std::vector<DeviceUnit>& units);

	// Gets the device unit list of the group
	const std::vector<DeviceUnit>& GetDeviceUnits() const 
	{
		return deviceUnits;
	}

	void SetGroupNum(int num) 
	{
		groupNum = num;
	}

	int GetTypeHash() const 
	{
		return typeHash;
	}

	void FlipGroupRotation();

	std::pair<std::string, CellRotation> GetFirstDeviceUnitWhoAndRotation();
	std::pair<std::string, CellRotation> GetLastDeviceUnitWhoAndRotation();

	bool HasDummyUnit() const 
	{
		return dummyNum > 0;
	}

private:
	//std::string name;
	std::vector<DeviceUnit> deviceUnits;
	int typeHash = 0;
	int groupNum = 0;

	int dummyNum = 0;

	// Recalculates the type hash based on the current device units, called whenever device units are modified
	void CalculateTypeHash();
};