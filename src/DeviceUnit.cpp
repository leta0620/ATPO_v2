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

std::vector<std::string> DeviceUnit::GetPatternUseNameList()
{
	std::vector<std::string> instNames;
	
	if (instName == "*") {
		for (int i = 0; i < width; ++i) 
		{
			instNames.push_back(this->instName);
		}
		
	}
	else {
		instNames.push_back(instName);
	}

	return instNames;
}