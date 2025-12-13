#pragma once
#include <vector>
#include <string>
#include <unordered_map>

class NetlistUnit
{
public:
	NetlistUnit() = default;
	NetlistUnit(const std::string& cellName, const std::string& synbolName, int deviceUnitCount, const std::string& analogType, int deviceWidth)
		: cellName(cellName), synbolName(synbolName), deviceUnitCount(deviceUnitCount), analogType(analogType), deviceWidth(deviceWidth)
	{;}

	void SetCellName(const std::string& name) { this->cellName = name; }
	std::string GetCellName() const { return this->cellName; }

	void SetSynbolName(const std::string& name) { this->synbolName = name; }
	std::string GetSynbolName() const { return this->synbolName; }

	void SetDeviceUnitCount(int count) { this->deviceUnitCount = count; }
	int GetDeviceUnitCount() const { return this->deviceUnitCount; }

	void SetAnalogType(const std::string& type) { this->analogType = type; }
	std::string GetAnalogType() const { return this->analogType; }

	void SetDeviceWidth(int width) { this->deviceWidth = width; }
	int GetDeviceWidth() const { return this->deviceWidth; }

	// pin name is [d, g, s]
	void AddPin(const std::string& synbolName, const std::string& pinD, const std::string& pinG, const std::string& pinS);
	void AddPinD(const std::string& synbolName, const std::string& pinD);
	void AddPinG(const std::string& synbolName, const std::string& pinG);
	void AddPinS(const std::string& synbolName, const std::string& pinS);

	// return pin name is [d, g, s]
	std::vector<std::string> GetPins(const std::string& synbolName) const;
	std::string GetPinD(const std::string& synbolName) const;	
	std::string GetPinG(const std::string& synbolName) const;
	std::string GetPinS(const std::string& synbolName) const;

private:
	std::string cellName, synbolName, analogType;
	int deviceUnitCount, deviceWidth;

	std::unordered_map<std::string, std::vector<std::string>> pinMap; // key(synbolName) -> , [d, g, s]
};