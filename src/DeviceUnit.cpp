#include "DeviceUnit.h"


std::string DeviceUnit::GetStringRotation() const
{
	if (rotation == CellRotation::R0) {
		return "R0";
	}
	else if (rotation == CellRotation::MY) {
		return "MY";
	}
	else {
		return "ERROR";
	}
}