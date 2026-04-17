#pragma once
#include "SystemStruct.h"

std::string CleanupNameForCode(std::string original);
std::string CleanupEqsForCode(std::string original);
int findingParameters(systemStruct* systemData);
int checkForSignalVars(systemStruct* systemData);
void mainDataProcess(systemStruct* systemData);