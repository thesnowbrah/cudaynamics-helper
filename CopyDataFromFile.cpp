#pragma once
#include "CopyDataFromFile.h"

std::string CleanupNameForCode(std::string original) {
	std::string result = original;
	for (int letter = 0; letter < result.size(); letter++) {
		if (result[letter] == 32 || result[letter] == 45) result[letter] = 95;
		else if (result[letter] > 64 && result[letter] < 91)result[letter] += 32; 
	}
	return result;
}

std::string CleanupEqsForCode(std::string original) {
	std::string result = "";
	std::string strTemp;

	int replacingPow = 0;

	if (original == "signal") return "signal";

	if (original[0] > 47 && original[0] < 58) { result = "(numb)"; result += original[0]; }
	else { result = original[0]; }
	for (int i = 1; i < original.size(); i++) {
		if ((original[i - 1] != 46 && original[i - 1] != 91 && (original[i - 1] < 48 || original[i - 1] > 57) && (original[i - 1] < 65 || original[i - 1] > 90) && (original[i - 1] < 97 || original[i - 1] > 122)) && (original[i] > 47 && original[i] < 58)) {
			result += "(numb)"; result += original[i];
		}
		else result += original[i];
		/*if (i >= 3) {
			if (result[i]=='(' && result[i-1] == 'w'&& result[i-2] == 'o'&& result[i-3] == 'p') {
				replacingPow = 0;
				strTemp = "";
				for (int j = i; j < original.size(); j++) {
					if (original[j] == '(')replacingPow++;
					else if (original[j] == ')')replacingPow--;
					else if (original[j] != ',')strTemp += original[j];
					else if (original[j] == ',' && replacingPow == 1) {
						result[i - 1] = 'g';
						result[i - 2] == 'o';
						result[i - 3] == 'l';
						result += strTemp;
						result += ") * ";
						strTemp = "";
					}
					else if (original[j] == ')' && replacingPow == 0) {
						result += "exp(";
						result += strTemp;
						result += ")";
						strTemp = "";
					}
				}
			}
		}*/
	}
	return result;
}

int findingParameters(systemStruct *systemData) {

	bool signalFound = false;
	for (int i = 0; i < systemData->varEqs.size(); i++) {

		std::string tempStr;
		int tempCounter = 0;
		bool timeToCheck = false;
		if (systemData->varEqs[i] == "signal") {
			continue;
		}
		for (int j = 0; j < systemData->varEqs[i].size(); j++) {
			
			if (tempCounter == 0) {
				if ((systemData->varEqs[i][j]>96 && systemData->varEqs[i][j] < 123) || (systemData->varEqs[i][j] > 64 && systemData->varEqs[i][j] < 91)) {
					tempCounter++;
					tempStr = systemData->varEqs[i][j];
				}

			}
			else {
				if ((systemData->varEqs[i][j] > 96 && systemData->varEqs[i][j] < 123) || (systemData->varEqs[i][j] > 64 && systemData->varEqs[i][j] < 91) || (systemData->varEqs[i][j] > 47 && systemData->varEqs[i][j] < 58) || systemData->varEqs[i][j] == 95) {
					tempCounter++;
					tempStr += systemData->varEqs[i][j];
				}
				else {
					timeToCheck = true;
					tempCounter = 0;
				}
			}

			if (timeToCheck || j == systemData->varEqs[i].size() - 1) {
				if (tempStr!="log" && tempStr != "fabs" && tempStr != "exp" && tempStr != "pow" && tempStr != "sin" && tempStr != "cos" && tempStr != "fmod" && tempStr != "signal" && tempStr != "fmin" && tempStr != "fmax" && tempStr != "min" && tempStr != "max" && tempStr != "abs" ) {
					bool isVarOrPar = false;
					for (int var = 0; var < systemData->varNames.size(); var++) {
						if (tempStr == systemData->varNames[var]) { isVarOrPar = true; break; }
					}

					if(!isVarOrPar)
					for (int par = 0; par < systemData->parameters.size(); par++) {
						if (tempStr == systemData->parameters[par]) { isVarOrPar = true; break; }
					}

					if (!isVarOrPar && tempStr!="") { 
						systemData->parameters.push_back(tempStr); 
					}
				}
				else if(tempStr == "signal")systemData->parameters.push_back("signal_param");
				tempStr = "";
				tempCounter = 0;
				timeToCheck = false;
			}

		}

	}

	return 0;
}

int checkForSignalVars(systemStruct* systemData) {
	bool signalVarFound = false;
	for (int i = 0; i < systemData->varEqs.size(); i++) {
		if (systemData->varEqs[i] == "signal") {
			signalVarFound = true;
			systemData->parameters.push_back(systemData->varNames[i] + "dc");
			systemData->parameters.push_back(systemData->varNames[i] + "amp");
			systemData->parameters.push_back(systemData->varNames[i] + "freq");
			systemData->parameters.push_back(systemData->varNames[i] + "del");
			systemData->parameters.push_back(systemData->varNames[i] + "df");
		}
	}
	if (signalVarFound) {
		systemData->parameters.push_back("signal");
		systemData->varNames.push_back("t");
		systemData->varEqs.push_back("Time");
	}

	return 0;
}



void mainDataProcess(systemStruct* systemData) {
	(*systemData).systemNameCode = CleanupNameForCode(systemData->systemNameTXT);
	for (int i = 0; i < systemData->varEqs.size(); i++) {
		(*systemData).varNamesCode.push_back(CleanupNameForCode((*systemData).varNames[i]));
	}
	findingParameters(systemData);
	checkForSignalVars(systemData);
	for (int i = 0; i < systemData->varEqs.size(); i++) {
		(*systemData).varEqs[i] = CleanupEqsForCode((*systemData).varEqs[i]);
	}

}

