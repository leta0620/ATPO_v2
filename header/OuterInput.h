#pragma once

#include <unordered_map>
#include <vector>
#include <string>

class OuterInput {
public:
	OuterInput() = default;
	OuterInput(const std::string& cdlFilename, const std::string& patternFilename)
		: cdlFile(cdlFilename), patternFile(patternFilename) {
	}

	void SetCdlFile(const std::string& filename) { cdlFile = filename; }
	void SetPatternFile(const std::string& filename) { patternFile = filename; }
	void SetIntermidiateFile(const std::string& filename) { intermidiateFile = filename; }

	// function
	bool ParsePatternFile();
	bool ParseCdlFile();
	bool GenIntermidiateFile();

	std::unordered_map<std::string, std::string> GetLabelNameMapInstName() { return labelNameMapInstName; }
	std::unordered_map<std::string, std::string> GetInstNameMapLabelName() { return instNameMapLabelName; }

private:
	std::string cdlFile, patternFile, intermidiateFile;
	std::unordered_map<std::string, std::string> labelNameMapInstName; // label name -> inst name
	std::unordered_map<std::string, std::string> instNameMapLabelName; // inst name -> label name

	std::vector<std::vector<std::string>> initialPatternTable;	// initial pattern from pattern file
	std::vector<std::vector<std::string>> initialPatternRotationTable;	// initial pattern rotation from pattern file

	std::unordered_map<std::string, std::tuple<std::string, std::string, std::string, std::string, int>> instNameMapCellInformation; // inst name -> pin1, pin2, pin3, pin4, deviceNum for one cell Type

	struct InstStruct
	{
		std::string dNet, gNet, sNet, bNet;
		int m;
		std::string instName, labelName;
		std::string cellType;
	};

	std::vector<InstStruct> instStructList;
};