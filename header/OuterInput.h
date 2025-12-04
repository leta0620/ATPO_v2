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

	// function
	bool ParsePatternFile();
	bool ParseCdlFile();

private:
	std::string cdlFile, patternFile;
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
	};

	std::vector<InstStruct> instStructList;
};