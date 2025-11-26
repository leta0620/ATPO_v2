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
}

void Group::SetDeviceUnits(const std::vector<DeviceUnit>& units)
{
	this->deviceUnits = units;
	this->CalculateTypeHash();
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

std::pair<std::string, std::string> Group::GetFirstDeviceUnitWhoAndOuterPin()
{
	if (deviceUnits.empty()) {
		return { "", "" };
	}

	DeviceUnit firstUnit = deviceUnits[0];
	if (firstUnit.GetRotation() == CellRotation::R0) {
		return { firstUnit.GetSymbol(), "D" };
	}
	else if (firstUnit.GetRotation() == CellRotation::MY) {
		return { firstUnit.GetSymbol(), "S" };
	}
	else {
		return { "", "" };
	}
}


std::pair<std::string, std::string> Group::GetLastDeviceUnitWhoAndOuterPin()
{
	if (deviceUnits.empty()) {
		return { "", "" };
	}
	DeviceUnit lastUnit = deviceUnits.back();
	if (lastUnit.GetRotation() == CellRotation::R0) {
		return { lastUnit.GetSymbol(), "S" };
	}
	else if (lastUnit.GetRotation() == CellRotation::MY) {
		return { lastUnit.GetSymbol(), "D" };
	}
	else {
		return { "", "" };
	}
}
