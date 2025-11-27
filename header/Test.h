#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "TableManager.h"
#include "DeviceUnit.h"
#include "InitialPlacement.h"
#include "SAManager.h"
#include "IntermidiateParser.h"


class Test
{
public:
	Test();

private:
	// Add your test functions here, and call them in the constructor
	void TestParsing();
	void TestInitialPlacement(int groupSize, int rowNum);
};