#include <string>
#include <vector>
#include <unordered_map>

class NetListLookupTable
{
public:
	NetListLookupTable() = default;
	NetListLookupTable(const std::string& cellName,	const std::string& synbolName, int deviceUnitCount,	const std::string& analogCellType)
		: cellName(cellName), synbolName(synbolName), deviceUnitCount(deviceUnitCount), analogCellType(analogCellType)
	{ ;	}

	void SetCellName(const std::string& name) { this->cellName = name; }
	std::string GetCellName() const { return this->cellName; }

	void SetSynbolName(const std::string& name) { this->synbolName = name; }
	std::string GetSynbolName() const { return this->synbolName; }

	void SetDeviceUnitCount(int count) { this->deviceUnitCount = count; }
	int GetDeviceUnitCount() const { return this->deviceUnitCount; }

	void SetAnalogCellType(const std::string& type) { this->analogCellType = type; }
	std::string GetAnalogCellType() const { return this->analogCellType; }

	void AddS(std::string);
	void AddG(std::string);
	void AddD(std::string);
	tuple<std::string, std::string, std::string> GetTerminals(const std::string& key) const;

private:
	string cellName;
	string synbolName;
	int deviceUnitCount;
	string analogCellType;

	std::unordered_map<std::string, tuple<std::string, std::string, std::string>> lookupTable; // key -> (d, g, s)
};