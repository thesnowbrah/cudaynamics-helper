#include "Delete.h"


void DeletePrep(std::filesystem::path cudaynamicsPath, std::vector<std::string> *systemNames, std::vector<std::string> *systemNamesCode) {
    systemNamesCode->clear();
    systemNames->clear();
	std::filesystem::path systems_dir = cudaynamicsPath / "systems";
    for (const auto& entry : std::filesystem::directory_iterator(systems_dir)) {
        if (entry.is_directory()) {
            std::string folder_name = entry.path().filename().string();
            std::filesystem::path base_path = systems_dir / folder_name / folder_name;
            systemNamesCode->push_back(folder_name);

            std::filesystem::path txt_path = base_path.string() + ".txt";
            
            std::ifstream txt_file(txt_path);
            if (txt_file.is_open()) {
                std::string line;
                std::getline(txt_file, line);
                
                systemNames->push_back(line.substr(6));
                txt_file.close();
            }


        }
    }

}

void DeleteSystem(std::filesystem::path cudaynamicsPath, std::filesystem::path systemPath, std::string systemNameCode) {
    std::filesystem::remove_all(systemPath);
	DeleteFromMainCPP(systemNameCode, cudaynamicsPath);
	DeleteFromSystemHeaders(systemNameCode, cudaynamicsPath);
	DeleteFromCudaynamicsProjFilters(systemNameCode, cudaynamicsPath);
	DeleteFromVCXPROJFile(systemNameCode, cudaynamicsPath);
}

void DeleteFromMainCPP(std::string systemNameCode, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "main.cpp").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string searchedLine = "(" + systemNameCode + ")";
	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find(searchedLine) == std::string::npos) {
			OutputTXT << line << '\n';
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);

}

void DeleteFromSystemHeaders(std::string systemNameCode, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "systemsHeaders.h").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string searchedLine = "/" + systemNameCode + "/";
	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find(searchedLine) == std::string::npos) {
			OutputTXT << line << '\n';
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);

}

void DeleteFromVCXPROJFile(std::string systemNameCode, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "CUDAynamics.vcxproj").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string searchedLine = "\\" + systemNameCode + "\\";
	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find(searchedLine) == std::string::npos) {
			OutputTXT << line << '\n';
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);
}

void DeleteFromCudaynamicsProjFilters(std::string systemNameCode, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "CUDAynamics.vcxproj.filters").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string searchedLine = "\\" + systemNameCode + "\\";
	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find(searchedLine) == std::string::npos) {
			OutputTXT << line << '\n';
		}
		else {
			std::getline(InputCPP, line); std::getline(InputCPP, line);
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);
}