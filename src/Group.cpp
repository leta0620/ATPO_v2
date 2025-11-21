#include "Group.h"

Group::Group(const std::string& name, int groupNum)
{
	this->name = name;
	this->groupNum = groupNum;

	this->typeHash = 0;
}

std::string Group::GetName()
{
	return name;
}
void Group::SetName(const std::string& n)
{
	name = n;
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
