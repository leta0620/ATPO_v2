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

	std::vector<std::string> initialPatternTable;	// initial pattern from pattern file
	std::vector<std::string> initialPatternRotationTable;	// initial pattern rotation from pattern file
};