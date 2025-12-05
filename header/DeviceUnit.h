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

	std::string GetStringRotation(bool leftS = true) const;

	// == operators
	bool operator==(const DeviceUnit& other) const {
		return (symbol == other.symbol);
	}



private:
	std::string symbol;
	std::string analogCellType;

	//DeviceUnit* d = nullptr, *g = nullptr, *s = nullptr;
	std::vector<std::string> d, g, s;
	
	CellRotation rotation = CellRotation::R0;
};