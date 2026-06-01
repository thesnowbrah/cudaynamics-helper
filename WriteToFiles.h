#pragma once
#include <fstream>

#include <filesystem>
#include <windows.h> // Äëÿ GetModuleFileName
#include "SystemStruct.h"

void WriteMain(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteTXT(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteHFile(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteCuFile(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteToMainCPP(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteToSystemHeaders(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteToCudaynamicsProjFilters(systemStruct systemData, std::filesystem::path cudaynamicsPath);
void WriteToVCXPROJFile(systemStruct systemData, std::filesystem::path cudaynamicsPath);

std::string ChangeEqsToKernelExplicitEuler(systemStruct systemData, std::string original);
std::string ChangeEqsToKernelDopri(systemStruct systemData, std::string original);
std::string ChangeEqsToKernelExplicitMidpoint(systemStruct systemData, std::string original);
std::string ChangeEqsToKernelExplicitMidpointForTMP(systemStruct systemData, std::string original, int eqNum);
std::string ChangeEqsToKernelSemiExplicit(systemStruct systemData, std::string original, int eqNum);
std::string ChangeEqsToKernelSemiExplicitForTMP(systemStruct systemData, std::string original, int eqNum);
std::string ChangeEqsToSolveImplicitness(systemStruct systemData, std::string original, bool* simpleIterations, int eqNum);