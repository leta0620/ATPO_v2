#include "Test.h"
#include <string>
#include <vector>

using namespace std;

Test::Test()
{
	// Add your test functions here, and call them in the constructor
	// this->TestParsing();
}

void Test::TestParsing()
{
	string intermediate_code_file_path = "test.txt";
	IntermidiateParser parser(intermediate_code_file_path);

	parser.Parse();
	parser.GenerateNetlistLookupTable();

	NetlistLookupTable& netlist = parser.GetNetlistLookupTable();
	// test get pin
	pair<string, string> dWho =  netlist.GetPinDLinkWho("A");
	pair<string, string> gWho = netlist.GetPinGLinkWho("A");
	pair<string, string> sWho = netlist.GetPinSLinkWho("A");

	pair<string, string> dWho1 = netlist.GetPinDLinkWho("B");
	pair<string, string> gWho1 = netlist.GetPinGLinkWho("B");
	pair<string, string> sWho1 = netlist.GetPinSLinkWho("B");

	return;
}