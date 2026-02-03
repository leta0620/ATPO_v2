#include "Group.h"

Group::Group(int groupNum)
{
	this->groupNum = groupNum;

	this->typeHash = 0;
}


void Group::AddDeviceUnit(const DeviceUnit& deviceUnit)
{
	this->deviceUnits.push_back(deviceUnit);
	this->CalculateTypeHash();

	if (deviceUnit.GetSymbol() == "d")
	{
		this->dummyNum++;
	}
}

void Group::SetDeviceUnits(const std::vector<DeviceUnit>& units)
{
	this->deviceUnits = units;
	this->CalculateTypeHash();

	for (const auto& unit : units)
	{
		if (unit.GetSymbol() == "d")
		{
			this->dummyNum++;
		}
	}
}

void Group::CalculateTypeHash()
{
	std::string concatenatedTypes;
	for (const auto& unit : this->deviceUnits)
	{
		concatenatedTypes += unit.GetAnalogCellType();
	}

	this->typeHash = int(std::hash<std::string>{}(concatenatedTypes));
}

std::pair<std::string, CellRotation> Group::GetFirstDeviceUnitWhoAndRotation()
{
	if (deviceUnits.empty()) {
		return { "", CellRotation::ERROR };
	}

	DeviceUnit firstUnit = deviceUnits[0];
	if (firstUnit.GetRotation() == CellRotation::R0) {
		return { firstUnit.GetSymbol(), CellRotation::R0 };
	}
	else if (firstUnit.GetRotation() == CellRotation::MY) {
		return { firstUnit.GetSymbol(), CellRotation::MY };
	}
	else {
		return { "", CellRotation::ERROR };
	}
}


std::pair<std::string, CellRotation> Group::GetLastDeviceUnitWhoAndRotation()
{
	if (deviceUnits.empty()) {
		return { "", CellRotation::ERROR };
	}
	DeviceUnit lastUnit = deviceUnits.back();
	if (lastUnit.GetRotation() == CellRotation::R0) {
		return { lastUnit.GetSymbol(), CellRotation::R0 };
	}
	else if (lastUnit.GetRotation() == CellRotation::MY) {
		return { lastUnit.GetSymbol(), CellRotation::MY };
	}
	else {
		return { "", CellRotation::ERROR };
	}
}

void Group::FlipGroupRotation()
{
	std::vector<DeviceUnit> newDeviceUnits;
	while (!deviceUnits.empty())
	{
		DeviceUnit unit = deviceUnits.back();
		deviceUnits.pop_back();
		unit.FlipRotation();
		newDeviceUnits.push_back(unit);
	}
	this->deviceUnits = newDeviceUnits;
}