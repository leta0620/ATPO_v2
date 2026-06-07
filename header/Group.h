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

	int GetSymbolNameSequenceHash() const;

	std::string GetSymbolNameSequence();

	void FlipGroupRotation();

	std::pair<std::string, CellRotation> GetFirstDeviceUnitWhoAndRotation();
	std::pair<std::string, CellRotation> GetLastDeviceUnitWhoAndRotation();

	bool HasDummyUnit() const 
	{
		return dummyNum > 0;
	}

	bool CheckAllDummyUnit() const 
	{
		return dummyNum == deviceUnits.size();
	}

	void BuildAllDummyGroup(int groupSize);

	bool operator== (const Group& other) const 
	{
		if (deviceUnits.size() != other.deviceUnits.size()) {
			return false;
		}
		for (size_t i = 0; i < deviceUnits.size(); ++i) {
			if (!(deviceUnits[i] == other.deviceUnits[i])) {
				return false;
			}
		}
		return true;
	}

	void SetDummyPosition(int pos) 
	{
		if (pos < 0 || pos >= deviceUnits.size()) {
			std::cerr << "Error: Dummy position out of range." << std::endl;
			return;
		}

		if (deviceUnits[pos].GetSymbol() == "d") {
			std::cerr << "Warning: Position " << pos << " is already a dummy unit." << std::endl;
			return;
		}

		deviceUnits[pos].SetSymbol("d");
		dummyNum++;
		CalculateTypeHash();
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

namespace std
{
	template <>
	struct hash<Group>
	{
		size_t operator()(const Group& g) const noexcept
		{
			return static_cast<size_t>(g.GetSymbolNameSequenceHash());
		}
	};
}