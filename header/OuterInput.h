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
	std::vector<std::string> labelNameList;
	std::unordered_map<std::string, int> labelNameToInstNme;


};