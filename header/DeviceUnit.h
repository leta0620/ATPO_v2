#pragma once
#include <iostream>
#include <vector>
#include <string>

class DeviceUnit
{
public:
	DeviceUnit() = default;
	DeviceUnit(const std::string& name, const std::string& analogCellType)
		: name(name), analogCellType(analogCellType) {
	}
	std::string GetName() const {
		return name;
	}
	std::string GetAnalogCellType() const {
		return analogCellType;
	}

	void SetAnalogCellType(const std::string& type) {
		analogCellType = type;
	}
	void SetName(const std::string& n) {
		name = n;
	}

private:
	std::string name;
	std::string analogCellType;
};