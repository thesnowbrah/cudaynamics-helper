#pragma once
#include <string>
#include <vector>

struct systemStruct {
	std::string systemNameTXT;
	std::string systemNameCode;
	std::vector<std::string> varNames;
	std::vector<std::string> varNamesCode;
	std::vector<std::string> varEqs;
	std::vector<std::string> parameters;
	std::vector<bool> isDerivative;
	bool methodsBool[6];

	systemStruct() 
	{

	}

};