#pragma once
#include <iostream>
#include <vector>
#include <string>

enum class CellRotation {
	R0,
	MY,
};

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

	void SetRotation(CellRotation dir) {
		rotation = dir;
	}
	CellRotation GetRotation() const {
		return rotation;
	}

private:
	std::string name;
	std::string analogCellType;
	
	CellRotation rotation = CellRotation::R0;
};