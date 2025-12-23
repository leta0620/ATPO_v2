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
	DeviceUnit(const std::string& symbol, const std::string& instName, const std::string& analogCellType, int width)
		: symbol(symbol), instName(instName), analogCellType(analogCellType), width(width) {
	}
	std::string GetSymbol() const {
		return symbol;
	}
	std::string GetAnalogCellType() const {
		return analogCellType;
	}

	std::string GetInstName() const {
		return instName;
	}
	void SetInstName(const std::string& name) {
		instName = name;
	}

	int GetWidth() const {
		return width;
	}
	void SetWidth(int w) {
		width = w;
	}

	void SetAnalogCellType(const std::string& type) {
		analogCellType = type;
	}
	void SetSymbol(const std::string& symbol) {
		this->symbol = symbol;
		if (this->symbol == "d") {
			this->instName = "*";
		}
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

	std::vector<std::string> GetPatternUseNameList();


	// == operators
	bool operator==(const DeviceUnit& other) const {
		return (symbol == other.symbol);
	}



private:
	std::string symbol;
	std::string instName;
	std::string analogCellType;
	int width = 0;

	//DeviceUnit* d = nullptr, *g = nullptr, *s = nullptr;
	std::vector<std::string> d, g, s;
	
	CellRotation rotation = CellRotation::R0;
};