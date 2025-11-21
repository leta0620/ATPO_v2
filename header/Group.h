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
	Group(const std::string& name, int groupNum);

	// Gets the name of the group
	std::string GetName();
	// Sets the name of the group
	void SetName(const std::string& n);

	// Adds a device unit to the group and recalculates the type hash
	void AddDeviceUnit(const DeviceUnit& deviceUnit);
	// Sets the device units of the group and recalculates the type hash
	void SetDeviceUnits(const std::vector<DeviceUnit>& units);
	// Gets the type hash of the group
	const std::vector<DeviceUnit>& GetDeviceUnits() const 
	{
		return deviceUnits;
	}

private:
	std::string name;
	std::vector<DeviceUnit> deviceUnits;
	int typeHash = 0;
	int groupNum = 0;

	// Recalculates the type hash based on the current device units, called whenever device units are modified
	void CalculateTypeHash();
};