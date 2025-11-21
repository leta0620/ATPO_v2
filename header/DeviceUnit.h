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

	void SetConnections(DeviceUnit* drain, DeviceUnit* gate, DeviceUnit* source) {
		d = drain;
		g = gate;
		s = source;
	}

	void SetDrainPin(DeviceUnit* drain) {
		d = drain;
	}

	void SetGatePin(DeviceUnit* gate) {
		g = gate;
	}

	void SetSourcePin(DeviceUnit* source) {
		s = source;
	}

	DeviceUnit* GetDrainPin() const {
		return d;
	}

	DeviceUnit* GetGatePin() const {
		return g;
	}

	DeviceUnit* GetSourcePin() const {
		return s;
	}

private:
	std::string name;
	std::string analogCellType;

	DeviceUnit* d = nullptr, *g = nullptr, *s = nullptr;
	
	CellRotation rotation = CellRotation::R0;
};