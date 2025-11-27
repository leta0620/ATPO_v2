#pragma once
#include <iostream>
#include <vector>
#include <string>

enum class CellRotation {
	R0,
	MY,
	ERROR,
};

class DeviceUnit
{
public:
	DeviceUnit() = default;
	DeviceUnit(const std::string& symbol, const std::string& analogCellType)
		: symbol(symbol), analogCellType(analogCellType) {
	}
	std::string GetSymbol() const {
		return symbol;
	}
	std::string GetAnalogCellType() const {
		return analogCellType;
	}

	void SetAnalogCellType(const std::string& type) {
		analogCellType = type;
	}
	void SetSymbol(const std::string& symbol) {
		this->symbol = symbol;
	}

	void SetRotation(CellRotation dir) {
		rotation = dir;
	}
	CellRotation GetRotation() const {
		return rotation;
	}
	void FlipRotation() {
		if (rotation == CellRotation::R0) {
			rotation = CellRotation::MY;
		}
		else {
			rotation = CellRotation::R0;
		}
	}

	std::string GetStringRotation() const;

	// == operators
	bool operator==(const DeviceUnit& other) const {
		return (symbol == other.symbol);
	}

	/*void SetConnections(DeviceUnit* drain, DeviceUnit* gate, DeviceUnit* source) {
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
	}*/

private:
	// synbol 沒寫

	std::string symbol;
	std::string analogCellType;

	//DeviceUnit* d = nullptr, *g = nullptr, *s = nullptr;
	// 共端的識別、連接關係腳位標示
	std::vector<std::string> d, g, s;
	
	CellRotation rotation = CellRotation::R0;
};