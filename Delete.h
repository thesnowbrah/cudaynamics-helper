#pragma once
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>


void DeletePrep(std::filesystem::path cudaynamicsPath, std::vector<std::string> *systemNames, std::vector<std::string> *systemNamesCode);
void DeleteSystem(std::filesystem::path cudaynamicsPath, std::filesystem::path systemPath, std::string systemNameCode);
void DeleteFromMainCPP(std::string systemNameCode, std::filesystem::path cudaynamicsPath);
void DeleteFromSystemHeaders(std::string systemNameCode, std::filesystem::path cudaynamicsPath);
void DeleteFromVCXPROJFile(std::string systemNameCode, std::filesystem::path cudaynamicsPath);
void DeleteFromCudaynamicsProjFilters(std::string systemNameCode, std::filesystem::path cudaynamicsPath);
