#pragma once
#include <fstream>
#include "SystemStruct.h"

void WriteTXT(systemStruct systemData);
void WriteHFile(systemStruct systemData);
void WriteCuFile(systemStruct systemData);

std::string ChangeEqsToKernelExplicitEuler(systemStruct systemData, std::string original);
std::string ChangeEqsToKernelExplicitMidpoint(systemStruct systemData, std::string original);
std::string ChangeEqsToKernelSemiExplicit(systemStruct systemData, std::string original, int eqNum);