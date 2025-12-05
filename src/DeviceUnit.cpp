#include "DeviceUnit.h"


std::string DeviceUnit::GetStringRotation(bool leftS) const
{
	// default left is D in inter computation
	if (rotation == CellRotation::R0) {
		if (leftS) {
			return "MY";
		}
		else {
			return "R0";
		}
	}
	else if (rotation == CellRotation::MY) {
		if (leftS) {
			return "R0";
		}
		else {
			return "MY";
		}
	}
	else {
		return "ERROR";
	}
}