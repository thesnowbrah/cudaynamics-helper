#include "WriteToFiles.h"


void WriteMain(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::filesystem::path fullPath = cudaynamicsPath / "systems" / systemData.systemNameCode;
	std::filesystem::create_directory(fullPath);
	WriteTXT(systemData, fullPath);
	WriteCuFile(systemData, fullPath);
	WriteHFile(systemData, fullPath);
	WriteToMainCPP(systemData, cudaynamicsPath);
	WriteToSystemHeaders(systemData, cudaynamicsPath);
	WriteToVCXPROJFile(systemData, cudaynamicsPath);
	WriteToCudaynamicsProjFilters(systemData, cudaynamicsPath);
}

void WriteToMainCPP(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "main.cpp").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find("    selectKernel") != std::string::npos) {
			OutputTXT << "    addKernel(" << systemData.systemNameCode << ");\n";
			OutputTXT << line << '\n';
		}
		else {
			OutputTXT << line << '\n';
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);

}

void WriteToSystemHeaders(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "systemsHeaders.h").wstring();
	std::ofstream OutputTXT(tmpPath, std::ios::app);
	OutputTXT << "\n#include \"systems/"<<systemData.systemNameCode<<"/"<<systemData.systemNameCode<<".h\"";
	OutputTXT.close();

}

void WriteToVCXPROJFile(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "CUDAynamics.vcxproj").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find("<CudaCompile Include=\"gpu_variation.cu\" />") != std::string::npos) {
			OutputTXT << line << '\n';
			OutputTXT << "    <CudaCompile Include=\"systems\\" << systemData.systemNameCode << "\\" << systemData.systemNameCode << ".cu\" />\n";

		}
		else {
			OutputTXT << line << '\n';
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);
}

void WriteToCudaynamicsProjFilters(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::wstring tmpPath = (cudaynamicsPath / "CUDAynamics.vcxproj.filters").wstring();
	std::ofstream OutputTXT("temp/tempCPPfile.cpp");
	std::ifstream InputCPP(tmpPath);

	std::string line;
	while (std::getline(InputCPP, line)) {
		if (line.find("<CudaCompile Include=\"main.cu\" />") != std::string::npos) {
			OutputTXT << line << '\n';
			OutputTXT << "    <CudaCompile Include=\"systems\\" << systemData.systemNameCode << "\\" << systemData.systemNameCode << ".cu\">\n";
			OutputTXT << "      <Filter>systems</Filter>\n";
			OutputTXT << "    </CudaCompile>\n";

		}
		else {
			OutputTXT << line << '\n';
		}
	}

	InputCPP.close();
	OutputTXT.close();

	std::filesystem::copy_file("temp/tempCPPfile.cpp", tmpPath, std::filesystem::copy_options::overwrite_existing);
}

void WriteTXT(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::string tmp = systemData.systemNameCode + ".txt";
	std::ofstream OutputTXT(cudaynamicsPath / tmp);
	OutputTXT << "Name: " << systemData.systemNameTXT << " system\n" << "Steps: 10000\n"<< "Transient: 10000\n";
	OutputTXT << "// Defining step: \n"
		<< "// parameter/variable/discrete\n"
		<< "// Then, unless it is discrete, provide its name and define it like an attribute\n"
		<< "// If it is discrete, add nothing after\n";
	OutputTXT << "Step type: parameter h Fixed 0.01 0.1 0.01 100 0.0 0.0\n";
	OutputTXT << "// Compute on CUDAynamics launch: yes/no\n"
		<< "Execute on launch: yes\n"
		<< "// Ranging types:\n"
		<< "// Fixed (single <minimum value>)\n"
		<< "// Linear (<step count> values uniformly distributed between <minimum value> and <maximum value>, inclusively)\n"
		<< "// Step (values are picked from <minimum value> to <maximum value> with a step <step>, including minimum, including maximum if it ends up as a picked value)\n"
		<< "// Random (<step count> values randomly picked from <minimum value> to <maximum value>)\n"
		<< "// Normal (normal distribution of <step count> values, defined by <normal mean> and <normal deviation>\n"
		<< "//\n"
		<< "// Defining variables/parameters:\n"
		<< "// var/param <name> <ranging type> <minimum value> <maximum value> <step> <step count> <normal mean> <normal deviation>\n";

	bool hasSignal = false;
	for (int i = 0; i < systemData.varNames.size(); i++) {
		if (systemData.varEqs[i] == "signal") {
			hasSignal = true;
		}
	}

	for (int i = 0; i < systemData.varNames.size(); i++) {
		OutputTXT << "var " << systemData.varNames[i] << " Fixed 0.0 10.0 1.0 50 0.0 0.0\n";
	}
	for (int i = 0; i < systemData.parameters.size(); i++) {
		if(!hasSignal || systemData.parameters[i]!="signal")
			OutputTXT << "param " << systemData.parameters[i] << " Fixed 0.0 10.0 1.0 50 0.0 0.0\n";
	}
	//OutputTXT << "param symmetry Fixed 0.0 1.0 0.01 100 0.0 0.0\n";

	OutputTXT << "// Defining enumerated parameters (useful for methods):\n"
		<< "// enum <name> <ranging type> <minimum value> <maximum value> <step> <step count> <normal mean> <normal deviation> <enum names, no spaces>\n";
		
	
	if (hasSignal)OutputTXT << "enum signal 1square 0sine 0triangle\n";

	OutputTXT << "enum method "; 
	bool firstMethodFound = false;
	std::string methodStrs[5] = { "ExplicitEuler", "SemiExplicitEuler", "ExplicitMidpoint", "ExplicitRungeKutta4", "ExplicitDormandPrince8" };// NO VSCD
	for (int i = 1; i < 6; i++) {
		if (systemData.methodsBool[i] == true) {
			if (!firstMethodFound) { firstMethodFound = true; OutputTXT << "1"; }
			else { OutputTXT << " 0"; }
			OutputTXT << methodStrs[i - 1];
		}
	}
	OutputTXT << "\n";
	OutputTXT << "// Defining settings for analysis functions:\n"
		<< "// analysis <name from \"anfunc_names.cpp\"> settings <values, must exactly match the settings struct> \n"
		<< "analysis Minimum / maximum settings 2 2\n"
		<< "analysis Largest Lyapunov exponent settings 0.01 30 0 0 1 2 -1\n";

	OutputTXT.close();

}

void WriteHFile(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::string tmp = systemData.systemNameCode + ".h";
	std::ofstream OutputTXT(cudaynamicsPath / tmp);
	OutputTXT << "#pragma once\n#include <kernels_common.h>\n\n"
		<< "#define name " << systemData.systemNameCode << "\n\n"
		// << "const int THREADS_PER_BLOCK_(name) = 64;\n\n"
		<< "__global__ void gpu_wrapper_(name)(Computation* data, uint64_t variation);\n\n"
		<< "__host__ __device__ void kernelProgram_(name)(Computation* data, uint64_t variation);\n\n"
		<< "__host__ __device__ __forceinline__ void finiteDifferenceScheme_(name)(numb* currentV, numb* nextV, numb* parameters, PerThread* pt);\n\n"
		<< "#undef name\n\n";

	OutputTXT.close();
}

void WriteCuFile(systemStruct systemData, std::filesystem::path cudaynamicsPath) {
	std::string tmp = systemData.systemNameCode + ".cu";
	std::ofstream OutputTXT(cudaynamicsPath / tmp);
	OutputTXT << R"(#include ")" << systemData.systemNameCode << R"(.h")" << std::endl;
	OutputTXT << "#define name " << systemData.systemNameCode << "\n\n";

	OutputTXT << "namespace attributes\n{\n";

	OutputTXT << "enum variables { ";
	OutputTXT << systemData.varNames[0];
	for (int i = 1; i < systemData.varNames.size(); i++) {
		OutputTXT << ", " << systemData.varNames[i];
	}
	OutputTXT << " };\n";

	OutputTXT << "enum parameters { ";
	for (int i = 0; i < systemData.parameters.size(); i++) {
		OutputTXT <<  systemData.parameters[i] << ", ";
	}
	OutputTXT << "method, COUNT };\n"; //symmetry,

	bool hasSignal = false;

	for (int i = 0; i < systemData.varNames.size(); i++) {
		if (systemData.varEqs[i] == "signal") {
			hasSignal = true;
			break;
		}
	}

	if (hasSignal)OutputTXT << "enum waveforms { square, sine, triangle };\n";

	OutputTXT << "enum methods { ";
	bool first = true;
	std::string methodStrs[5] = { "ExplicitEuler", "SemiExplicitEuler", "ExplicitMidpoint", "ExplicitRungeKutta4", "ExplicitDormandPrince8" };// NO VSCD
	for (int i = 1; i < 6; i++) {
		if (systemData.methodsBool[i] == true) {
			if (first) { OutputTXT << methodStrs[i - 1]; first = false; }
			else { OutputTXT << ", " << methodStrs[i - 1]; }
		}
	}
	OutputTXT << "};\n}\n\n";

	OutputTXT << "__global__ void gpu_wrapper_(name)(Computation* data, uint64_t variation)\n"
		<< "{\n    kernelProgram_(name)(data, (blockIdx.x* blockDim.x) + threadIdx.x);\n}\n";

	OutputTXT << "__host__ __device__ void kernelProgram_(name)(Computation* data, uint64_t variation)\n{\n"
		<< "    if (variation >= CUDA_marshal.totalVariations) return;   // Shutdown thread if there isn't a variation to compute\n"
		<< "    uint64_t stepStart, variationStart = variation * CUDA_marshal.variationSize;         // Start index to store the modelling data for the variation\n"
		<< "    LOCAL_BUFFERS;\n    LOAD_ATTRIBUTES(false);\n"
		<< "    // Custom area (usually) starts here\n"
		<< "    TRANSIENT_SKIP_NEW(finiteDifferenceScheme_(name));\n"
		<< "    for (int s = 0; s < CUDA_kernel.steps && !data->isHires; s++)\n    {\n"
		<< "        stepStart = variationStart + s * CUDA_kernel.VAR_COUNT;\n"
		<< "        finiteDifferenceScheme_(name)(FDS_ARGUMENTS);\n        RECORD_STEP;\n    }\n\n"
		<< "    // Analysis\n    AnalysisLobby(data, &finiteDifferenceScheme_(name), variation);\n}\n";
		
	OutputTXT << "__host__ __device__ __forceinline__ void finiteDifferenceScheme_(name)(numb* currentV, numb* nextV, numb* parameters, PerThread* pt)\n{\n\n";
	
	int eqsNum = 0;
	for (int i = 0; i < systemData.varEqs.size(); i++) { if (systemData.varEqs[i] != "signal") { eqsNum++; } }
	OutputTXT << "    const int Number = " << eqsNum << ";\n";
	OutputTXT << "    numb v[Number] = {";
	int tmpVarCount = 0;
	int timeNum;
	for (int i = 0; i < eqsNum;) {
		if (systemData.varEqs[tmpVarCount] != "signal") {
			if (i > 0)OutputTXT << ", ";
			if (systemData.varEqs[tmpVarCount] == "Time")timeNum = i;
			OutputTXT << "V(" << systemData.varNames[tmpVarCount] << ")";
			i++; tmpVarCount++;
		}
		else { tmpVarCount++; }
	}
	OutputTXT << "};\n";
	int temp;
	if (hasSignal)temp = 3;
	else temp = 1;

	for (int signal = 0; signal < temp; signal++) {
		
		if (hasSignal && signal == 0) OutputTXT << "    ifSIGNAL(P(signal), square)\n    {\n";
		else if(signal == 1)OutputTXT << "    ifSIGNAL(P(signal), sine)\n    {\n";
		else if (signal == 2)OutputTXT << "    ifSIGNAL(P(signal), triangle)\n    {\n";

		if (systemData.methodsBool[0] == 1 || systemData.methodsBool[1] == 1 ) {
			//			EXPLICIT EULER
			OutputTXT << "    ifMETHOD(P(method), ExplicitEuler)\n    {\n";

			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((V(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(V(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - V(t))"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (V(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 *P( " << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}

			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H;\n";
				}

				else if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i])OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H * (" << ChangeEqsToKernelExplicitEuler(systemData, systemData.varEqs[i]) << ");\n";
					else if (systemData.varEqs[i] != "signal")OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = " << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}

			OutputTXT << "    }\n\n";
			//			EXPLICIT EULER
		}

		if (systemData.methodsBool[0] == 1 || systemData.methodsBool[2] == 1) {
			//			EXPLICIT EULER-CROMER
			OutputTXT << "    ifMETHOD(P(method), SemiExplicitEuler)\n    {\n";

			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((V(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(V(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - V(t))"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (V(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}

			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H;\n";
				}

				else if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i])OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H * (" << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ");\n";
					else  if (systemData.varEqs[i] != "signal")OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = " << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}

			OutputTXT << "    }\n\n";
			//			EXPLICIT EULER-CROMER
		}

		if (systemData.methodsBool[0] == 1 || systemData.methodsBool[3] == 1) {
			//			EXPLICIT MIDPOINT
			OutputTXT << "    ifMETHOD(P(method), ExplicitMidpoint)\n    {\n";

			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((V(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(V(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - V(t))"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (V(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        numb " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + H * (numb)0.5;\n";
				}

				else if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i])OutputTXT << "        numb " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + (numb)0.5 * H * (" << ChangeEqsToKernelExplicitEuler(systemData, systemData.varEqs[i]) << ");\n";
					else  if (systemData.varEqs[i] != "signal") OutputTXT << "        numb" << systemData.varNames[i] << "mp = " << ChangeEqsToKernelSemiExplicitForTMP(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}


			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((tmp - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(tmp - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - tmp)"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (tmp - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (tmp - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (tmp - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (tmp - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H;\n";
				}

				else if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i])OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H * (" << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ");\n";
					else  if (systemData.varEqs[i] != "signal")OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = " << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}

			OutputTXT << "    }\n\n";
			//			EXPLICIT MIDPOINT
		}

		if (systemData.methodsBool[0] == 1 || systemData.methodsBool[4] == 1) {
			//			EXPLICIT RK4
			OutputTXT << "    ifMETHOD(P(method), ExplicitRungeKutta4)\n    {\n";

			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((V(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(V(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - V(t))"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (V(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}

			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        numb k" << systemData.varNames[i] << "1 = 1;\n";
				}
				else if (systemData.varEqs[i] != "signal") {
					OutputTXT << "        numb k" << systemData.varNames[i] << "1 = " << ChangeEqsToKernelExplicitEuler(systemData, systemData.varEqs[i]) << ";\n";
				}
			}
			OutputTXT << "\n";
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i]) OutputTXT << "        numb " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + (numb)0.5 * H * k" << systemData.varNames[i] << "1;\n";
					else OutputTXT << "        numb" << systemData.varNames[i] << "mp = " << ChangeEqsToKernelSemiExplicitForTMP(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}
			OutputTXT << "\n";


			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((tmp - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(tmp - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - tmp)"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (tmp - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (tmp - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (tmp - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (tmp - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        numb k" << systemData.varNames[i] << "2 = 1;\n";
				}

				else if (systemData.varEqs[i] != "signal") {
					OutputTXT << "        numb k" << systemData.varNames[i] << "2 = " << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ";\n";
				}
			}
			OutputTXT << "\n";
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i])OutputTXT << "        " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + (numb)0.5 * H * k" << systemData.varNames[i] << "2;\n";
					else OutputTXT << "        " << systemData.varNames[i] << "mp = " << ChangeEqsToKernelExplicitMidpointForTMP(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}
			OutputTXT << "\n";

			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "Time" && systemData.varEqs[i] != "signal") OutputTXT << "        numb k" << systemData.varNames[i] << "3 = " << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ";\n";
				else if (systemData.varEqs[i] == "Time") OutputTXT << "        numb k" << systemData.varNames[i] << "3 = 1" << ";\n";

			}
			OutputTXT << "\n";
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i])OutputTXT << "        " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + H * k" << systemData.varNames[i] << "3;\n";
					else OutputTXT << "        " << systemData.varNames[i] << "mp = " << ChangeEqsToKernelExplicitMidpointForTMP(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}
			OutputTXT << "\n";


			if (hasSignal)
				for (int i = 0; i < systemData.varNames.size(); i++) {
					if (systemData.varEqs[i] == "signal") {
						if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((tmp - P(" << systemData.varNames[i] << "del)) > 0 ? "
							<< "(tmp - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - tmp)"
							<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
						if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
							<< " * P(" << systemData.varNames[i] << "freq) * (tmp - P(" << systemData.varNames[i] << "del)));\n";
						if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
							<< " * (tmp - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (tmp - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
							<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (tmp - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
					}
				}
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "Time") {
					OutputTXT << "        numb k" << systemData.varNames[i] << "4 = 1;\n";
				}
				else if (systemData.varEqs[i] != "signal") {
					OutputTXT << "        numb k" << systemData.varNames[i] << "4 = " << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ";\n";
				}
			}
			OutputTXT << "\n";

			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "signal")
				{
					if (systemData.isDerivative[i])
						OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ") + H * (k" << systemData.varNames[i] << "1"
						<< " + (numb)2.0 * k" << systemData.varNames[i] << "2" << " + (numb)2.0 * k" << systemData.varNames[i] << "3"
						<< " + k" << systemData.varNames[i] << "4) / (numb)6.0" << ";\n";
					else  if (systemData.varEqs[i] != "signal")
						OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = " << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i)<<";\n";
					else
						OutputTXT << "        " << systemData.varNames[i] << "mp = " << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ";\n";
				}
			}

			OutputTXT << "    }\n\n";
			//			EXPLICIT RK4
		}

		if (systemData.methodsBool[0] == 1 || systemData.methodsBool[5] == 1) {
			//			ExplicitDormandPrince8
			OutputTXT << "    ifMETHOD(P(method), ExplicitDormandPrince8)\n    {\n";

			OutputTXT << "			const numb M[13][12] = { {(numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.05555555555556, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.02083333333333, (numb)0.0625, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.03125, (numb)0.0, (numb)0.09375, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.3125, (numb)0.0, -(numb)1.171875, (numb)1.171875, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.0375, (numb)0.0, (numb)0.0, (numb)0.1875, (numb)0.15, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.04791013711111, (numb)0.0, (numb)0.0, (numb)0.1122487127778, -(numb)0.02550567377778, (numb)0.01284682388889, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.01691798978729, (numb)0.0, (numb)0.0, (numb)0.387848278486, (numb)0.0359773698515, (numb)0.1969702142157, -(numb)0.1727138523405, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.06909575335919, (numb)0.0, (numb)0.0, -(numb)0.6342479767289, -(numb)0.1611975752246, (numb)0.1386503094588, (numb)0.9409286140358, (numb)0.2116363264819, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.183556996839, (numb)0.0, (numb)0.0, -(numb)2.468768084316, -(numb)0.2912868878163, -(numb)0.02647302023312, (numb)2.847838764193, (numb)0.2813873314699, (numb)0.1237448998633, (numb)0.0, (numb)0.0, (numb)0.0},\n"
				<< "								{-(numb)1.215424817396, (numb)0.0, (numb)0.0, (numb)16.67260866595, (numb)0.9157418284168, -(numb)6.056605804357, -(numb)16.00357359416, (numb)14.8493030863, -(numb)13.37157573529, (numb)5.13418264818, (numb)0.0, (numb)0.0},\n"
				<< "								{(numb)0.2588609164383, (numb)0.0, (numb)0.0, -(numb)4.774485785489, -(numb)0.435093013777, -(numb)3.049483332072, (numb)5.577920039936, (numb)6.155831589861, -(numb)5.062104586737, (numb)2.193926173181, (numb)0.1346279986593, (numb)0.0},\n"
				<< "								{(numb)0.8224275996265, (numb)0.0, (numb)0.0, -(numb)11.65867325728, -(numb)0.7576221166909, (numb)0.7139735881596, (numb)12.07577498689, -(numb)2.12765911392, (numb)1.990166207049, -(numb)0.234286471544, (numb)0.1758985777079, (numb)0.0} };\n\n";

			OutputTXT << "			const numb b[13] = { (numb)0.04174749114153, (numb)0.0, (numb)0.0, (numb)0.0, (numb)0.0, -(numb)0.05545232861124, (numb)0.2393128072012, (numb)0.7035106694034, -(numb)0.7597596138145, (numb)0.6605630309223, (numb)0.1581874825101, -(numb)0.2381095387529, (numb)0.25 };\n";
			OutputTXT << "          numb y[Number], X1[Number], X2[Number];\n";
			OutputTXT << "          numb k[Number][13];\n";
			OutputTXT << "          int i = 0, j = 0, l = 0;\n";
			for (int i = 0; i < systemData.varEqs.size(); i++) {
				if (systemData.varEqs[i] == "signal")
					OutputTXT << "        numb " << systemData.varNames[i] << "mp;\n";
			}
			OutputTXT << "        for (i = 0; i < Number; i++){\n";
			OutputTXT << "            X1[i] = v[i];\n";
			OutputTXT << "        }\n";

			OutputTXT << "        for (i = 0; i < 13; i++){\n";
			for (int i = 0; i < systemData.varEqs.size(); i++) {
				if (systemData.varEqs[i] == "signal")
				{
					if (signal == 0) OutputTXT << "        " << systemData.varNames[i] << "mp = P(" << systemData.varNames[i] << "dc) + (fmod((X1[" << timeNum << "] - P(" << systemData.varNames[i] << "del)) > 0 ? "
						<< "(X1[" << timeNum << "] - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) -X1[" << timeNum << "])"
						<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
					if (signal == 1) OutputTXT << "        " << systemData.varNames[i] << "mp = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
						<< " * P(" << systemData.varNames[i] << "freq) * (X1[" << timeNum << "] - P(" << systemData.varNames[i] << "del)));\n";
					if (signal == 2) OutputTXT << "        " << systemData.varNames[i] << "mp = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
						<< " * (v[" << timeNum << "] - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (v[" << timeNum << "] - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
						<< " * ((int)floor(((numb)4.0 *P( " << systemData.varNames[i] << "freq) * (X1[" << timeNum << "] - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";

				}
			}
			for (int i = 0; i < systemData.varEqs.size(); i++) {
				if (!systemData.isDerivative[i] && systemData.varEqs[i] != "signal")
				{
					OutputTXT << "        numb " << systemData.varNames[i] << "mp = " << ChangeEqsToKernelDopri(systemData, systemData.varEqs[i]) << ";\n";

				}
			}
			int eqNumTmp = 0;
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "signal" && systemData.isDerivative[i]) {
					if (hasSignal) {
						if (i == systemData.varNames.size()-1)OutputTXT << "            k[" << eqNumTmp << "][i] = (numb)1.0;\n";
						else OutputTXT << "            k[" << eqNumTmp << "][i] = " << ChangeEqsToKernelDopri(systemData, systemData.varEqs[i]) << ";\n";
					}
					else {
						OutputTXT << "            k[" << eqNumTmp << "][i] = " << ChangeEqsToKernelDopri(systemData, systemData.varEqs[i]) << ";\n";
					}
					eqNumTmp++;
				}
			}
			OutputTXT << "				for (l = 0; l < Number; l++)\n 					X2[l] = 0;\n\n";
			OutputTXT << "				for (j = 0; j < i + 1; j++)\n					for (l = 0; l < Number; l++)\n						X2[l] += M[i + 1][j] * k[l][j];\n\n";
			OutputTXT << "				for (l = 0; l < Number; l++)\n					X1[l] = v[l] + H * X2[l];\n\n";

			OutputTXT << "          }\n\n";

			OutputTXT << "			for (l = 0; l < Number; l++)\n				X2[l] = 0;\n\n";
			OutputTXT << "			for (i = 0; i < 13; i++)\n				for (l = 0; l < Number; l++)\n					X2[l] += b[i] * k[l][i];\n\n";
			OutputTXT << "			for (l = 0; l < Number; l++)\n				y[l] = v[l] + H * X2[l];\n";

			eqNumTmp = 0;
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] != "signal") {
					if (systemData.isDerivative[i]) {
						OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = y[" << eqNumTmp << "]; \n";
						eqNumTmp++;
					}
					else {
						OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = "<< ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) <<"; \n";
					}
				}
			}
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "signal") {
					if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((Vnext(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
						<< "(Vnext(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - Vnext(t))"
						<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
					if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
						<< " * P(" << systemData.varNames[i] << "freq) * (Vnext(t) - P(" << systemData.varNames[i] << "del)));\n";
					if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
						<< " * (Vnext(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (Vnext(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
						<< " * ((int)floor(((numb)4.0 *P( " << systemData.varNames[i] << "freq) * (Vnext(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";

				}
			}

			OutputTXT << "    }\n\n";
			//			ExplicitDormandPrince8
		}
		/*
		//			 VSCD
		OutputTXT << "    ifMETHOD(P(method), VariableSymmetryCD)\n    {\n";
		OutputTXT << "        numb h1 = (numb)0.5 * H - P(symmetry);\n        numb h2 = (numb)0.5 * H + P(symmetry);\n";

		//			SEMI EXPLICIT EULER
		if (hasSignal)
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "signal") {
					if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((V(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
						<< "(V(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - V(t))"
						<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
					if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
						<< " * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)));\n";
					if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
						<< " * (V(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
						<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (V(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
				}
			}

		for (int i = 0; i < systemData.varNames.size(); i++) {
			if (systemData.varEqs[i] == "Time") {
				OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H;\n";
			}

			else if (systemData.varEqs[i] != "signal") {
				OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + h1 * (" << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ");\n";
			}
		}

		//			SEMI EXPLICIT EULER

		//			IMPLICITNESS
		if (hasSignal)
			for (int i = 0; i < systemData.varNames.size(); i++) {
				if (systemData.varEqs[i] == "signal") {
					if (signal == 0) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + (fmod((Vnext(t) - P(" << systemData.varNames[i] << "del)) > 0 ? "
						<< "(Vnext(t) - P(" << systemData.varNames[i] << "del)) : (P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) + P(" << systemData.varNames[i] << "del) - Vnext(t))"
						<< ", 1 / P(" << systemData.varNames[i] << "freq)) < P(" << systemData.varNames[i] << "df) / P(" << systemData.varNames[i] << "freq) ? P(" << systemData.varNames[i] << "amp) : (numb)0.0);\n";
					if (signal == 1) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * sin((numb)2.0 * (numb)3.141592653589793"
						<< " * P(" << systemData.varNames[i] << "freq) * (Vnext(t) - P(" << systemData.varNames[i] << "del)));\n";
					if (signal == 2) OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = P(" << systemData.varNames[i] << "dc) + P(" << systemData.varNames[i] << "amp) * (((numb)4.0 * P(" << systemData.varNames[i] << "freq)"
						<< " * (Vnext(t) - P(" << systemData.varNames[i] << "del)) - (numb)2.0 * floor(((numb)4.0 * P(" << systemData.varNames[i] << "del) * (Vnext(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0))"
						<< " * ((int)floor(((numb)4.0 * P(" << systemData.varNames[i] << "freq) * (Vnext(t) - P(" << systemData.varNames[i] << "del)) + (numb)1.0) / (numb)2.0) % 2 == 0 ? (numb)1.0 : (numb)-1.0));\n";
				}
			}
		for (int i = systemData.varNames.size() - 1; i >= 0; i--) {
			if(systemData.varEqs[i] != "Time" && systemData.varEqs[i] != "signal")
			{
			//		MAKE A FUNCTION THAT WOULD FIND AND SOLVE IMPLICITNESS
				
				bool simpleIterations = false;
				std::string solvedImplicitness = ChangeEqsToSolveImplicitness(systemData, systemData.varEqs[i], &simpleIterations, i);

				OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + h1 * (" << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ");\n";
			}
		}

		//			IMPLICITNESS

		OutputTXT << "    }\n\n";
		//			 VSCD
		*/

		if(hasSignal)OutputTXT<< "    }\n\n";

		
	}
	OutputTXT << "}\n";
	OutputTXT.close();
}

std::string ChangeEqsToKernelExplicitEuler(systemStruct systemData, std::string original) {
		std::string result = "";
		bool nameOrFunc = false;
		bool varOrParFound = false;
		std::string tempStr = "";
		for (int i = 0; i < original.size(); i++) {
			if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {
				
				if (nameOrFunc) {
					if (tempStr != "") {
						for (int var = 0; var < systemData.varNames.size(); var++) {
							if (tempStr == systemData.varNames[var]) {
								if (systemData.varEqs[var] == "signal") {
									result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
								}
								else {
									result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
								}
							}
						}
						for (int par = 0; par < systemData.parameters.size(); par++) {
							if (tempStr == systemData.parameters[par]) {
								result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
						}
					}
					if (!varOrParFound) {
						result += tempStr;
					}
					else varOrParFound = false;
					nameOrFunc = false;
					tempStr = "";
				}
				result += original[i];
			}
			else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
				nameOrFunc = true;
				tempStr += original[i];
				if (i == original.size() - 1) {
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (systemData.varEqs[var] == "signal") {
								result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
							else {
								result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
					if (!varOrParFound) {
						result += tempStr;
					}
					else varOrParFound = false;
				}

			}
		}
	
	return result;
}

std::string ChangeEqsToKernelDopri(systemStruct systemData, std::string original) {
	std::string result = "";
	bool nameOrFunc = false;
	bool varOrParFound = false;
	std::string tempStr = "";
	int tmpInt = 0;
	for (int i = 0; i < original.size(); i++) {
		if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {

			if (nameOrFunc) {
				if (tempStr != "") {
					
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (systemData.varEqs[var] == "signal" || !systemData.isDerivative[var]) {
								result += tempStr; result += "mp"; varOrParFound = true; break;
							}
							else {
								result += "X1["; result += std::to_string(tmpInt); result += "]"; varOrParFound = true; break;
							}
						}
						else { 
							if(systemData.varEqs[var]!="signal")tmpInt++; 
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
				tmpInt = 0;
				nameOrFunc = false;
				tempStr = "";
			}
			result += original[i];
		}
		else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
			nameOrFunc = true;
			tempStr += original[i];
			if (i == original.size() - 1) {
				for (int var = 0; var < systemData.varNames.size(); var++) {
					if (tempStr == systemData.varNames[var]) {
						if (systemData.varEqs[var] == "signal" || !systemData.isDerivative[var]) {
							result += tempStr; result += "mp"; varOrParFound = true; break;
						}
						else {
							result += "X1["; result += std::to_string(tmpInt); result += "]"; varOrParFound = true; break;
						}
					}
					else {
						if (systemData.varEqs[var] != "signal")tmpInt++;
					}
				}
				for (int par = 0; par < systemData.parameters.size(); par++) {
					if (tempStr == systemData.parameters[par]) {
						result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
			}

		}
	}

	return result;
}

std::string ChangeEqsToKernelExplicitMidpoint(systemStruct systemData, std::string original) {
	std::string result = "";
	bool nameOrFunc = false;
	bool varOrParFound = false;
	std::string tempStr = "";
	for (int i = 0; i < original.size(); i++) {
		if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {

			if (nameOrFunc) {
				if (tempStr != "") {
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (systemData.varEqs[var] == "signal") {
								result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
							else if (!systemData.isDerivative[var]) {
								 result += tempStr; result += "mp"; varOrParFound = true; break;
							}
							else {
								result += tempStr; result += "mp"; varOrParFound = true; break;
							}
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
				nameOrFunc = false;
				tempStr = "";
			}
			result += original[i];
		}
		else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
			nameOrFunc = true;
			tempStr += original[i];
			if (i == original.size() - 1) {
				for (int var = 0; var < systemData.varNames.size(); var++) {
					if (tempStr == systemData.varNames[var]) {
						if (systemData.varEqs[var] == "signal") {
							result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
						else if (!systemData.isDerivative[var]) {
							result += tempStr; result += "mp"; varOrParFound = true; break;
						}
						else {
							result += tempStr; result += "mp"; varOrParFound = true; break;
						}
					}
				}
				for (int par = 0; par < systemData.parameters.size(); par++) {
					if (tempStr == systemData.parameters[par]) {
						result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
			}

		}
	}

	return result;
}

std::string ChangeEqsToKernelExplicitMidpointForTMP(systemStruct systemData, std::string original, int eqNum) {
	std::string result = "";
	bool nameOrFunc = false;
	bool varOrParFound = false;
	std::string tempStr = "";
	for (int i = 0; i < original.size(); i++) {
		if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {

			if (nameOrFunc) {
				if (tempStr != "") {
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (systemData.varEqs[var] == "signal") {
								result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
							else if (!systemData.isDerivative[var]) {
								result += tempStr; result += "mp"; varOrParFound = true; break;
							}
							else {
								result += tempStr; result += "mp"; varOrParFound = true; break;
							}
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
				nameOrFunc = false;
				tempStr = "";
			}
			result += original[i];
		}
		else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
			nameOrFunc = true;
			tempStr += original[i];
			if (i == original.size() - 1) {
				for (int var = 0; var < systemData.varNames.size(); var++) {
					if (tempStr == systemData.varNames[var]) {
						if (systemData.varEqs[var] == "signal") {
							result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
						else if (!systemData.isDerivative[var]) {
							result += tempStr; result += "mp"; varOrParFound = true; break;
						}
						else {
							result += tempStr; result += "mp"; varOrParFound = true; break;
						}
					}
				}
				for (int par = 0; par < systemData.parameters.size(); par++) {
					if (tempStr == systemData.parameters[par]) {
						result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
			}

		}
	}

	return result;
}

std::string ChangeEqsToKernelSemiExplicit(systemStruct systemData, std::string original, int eqNum) {
	std::string result = "";
	bool nameOrFunc = false;
	bool varOrParFound = false;
	std::string tempStr = "";
	for (int i = 0; i < original.size(); i++) {
		if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {

			if (nameOrFunc) {
				if (tempStr != "") {
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (var >= eqNum || !systemData.isDerivative[var] && systemData.varEqs[var]!="signal") {
								result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
							else {
								result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
				nameOrFunc = false;
				tempStr = "";
			}
			result += original[i];
		}
		else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
			nameOrFunc = true;
			tempStr += original[i];
			if (i == original.size() - 1) {
				for (int var = 0; var < systemData.varNames.size(); var++) {
					if (tempStr == systemData.varNames[var]) {
						if (var >= eqNum || !systemData.isDerivative[var] && systemData.varEqs[var] != "signal") {
							result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
						else {
							result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				for (int par = 0; par < systemData.parameters.size(); par++) {
					if (tempStr == systemData.parameters[par]) {
						result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
			}

		}
	}

	return result;
}

std::string ChangeEqsToKernelSemiExplicitForTMP(systemStruct systemData, std::string original, int eqNum) {
	std::string result = "";
	bool nameOrFunc = false;
	bool varOrParFound = false;
	std::string tempStr = "";
	for (int i = 0; i < original.size(); i++) {
		if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {

			if (nameOrFunc) {
				if (tempStr != "") {
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (var >= eqNum || !systemData.isDerivative[var] && systemData.varEqs[var] != "signal") {
								result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
							else {
								result += tempStr; result += "mp"; varOrParFound = true; break;
							}
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
				nameOrFunc = false;
				tempStr = "";
			}
			result += original[i];
		}
		else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
			nameOrFunc = true;
			tempStr += original[i];
			if (i == original.size() - 1) {
				for (int var = 0; var < systemData.varNames.size(); var++) {
					if (tempStr == systemData.varNames[var]) {
						if (var >= eqNum || !systemData.isDerivative[var] && systemData.varEqs[var] != "signal") {
							result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
						else {
							 result += tempStr; result += "mp"; varOrParFound = true; break;
						}
					}
				}
				for (int par = 0; par < systemData.parameters.size(); par++) {
					if (tempStr == systemData.parameters[par]) {
						result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
			}

		}
	}

	return result;
}

std::string ChangeEqsToSolveImplicitness(systemStruct systemData, std::string original, bool* simpleIterations, int eqNum) {
	std::string result = "";
	bool nameOrFunc = false;
	bool varOrParFound = false;
	std::string tempStr = "";
	
	bool implicitnessFound = false;


	for (int i = 0; i < original.size(); i++) {
		if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {

			if (nameOrFunc) {
				if (tempStr != "") {
					for (int var = 0; var < systemData.varNames.size(); var++) {
						if (tempStr == systemData.varNames[var]) {
							if (var >= eqNum) {
								result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
							else {
								result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
							}
						}
					}
					for (int par = 0; par < systemData.parameters.size(); par++) {
						if (tempStr == systemData.parameters[par]) {
							result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
				nameOrFunc = false;
				tempStr = "";
			}
			result += original[i];
		}
		else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
			nameOrFunc = true;
			tempStr += original[i];
			if (i == original.size() - 1) {
				for (int var = 0; var < systemData.varNames.size(); var++) {
					if (tempStr == systemData.varNames[var]) {
						if (var == eqNum) {
							implicitnessFound = true;
						}
						else if (var > eqNum) {
							result += "Vnext("; result += tempStr; result += ")"; varOrParFound = true; break;
						}
						else {
							result += tempStr; result += "mp"; varOrParFound = true; break;
						}
					}
				}
				for (int par = 0; par < systemData.parameters.size(); par++) {
					if (tempStr == systemData.parameters[par]) {
						result += "P("; result += tempStr; result += ")"; varOrParFound = true; break;
					}
				}
				if (!varOrParFound) {
					result += tempStr;
				}
				else varOrParFound = false;
			}

		}
	}

	if (!implicitnessFound) {
		return result;
	}
	else {
		bool inFunc = false;
		varOrParFound = false;
		nameOrFunc = false;
		tempStr = "";
		int perenthesisCount = 0;


		//		THIS DOES NOT TAKE ACOUNT FOR POWERS OF THE VARIABLE DONE AS IN "X * X" OR  THINGS SUCH AS "1 / X"   /// NEED TO ADD CHECK
		for (int i = 0; i < original.size(); i++) {
			if (original[i] < 48 || (original[i] >= 48 && original[i] <= 57 && !nameOrFunc) || (original[i] <= 64 && original[i] >= 58) || (original[i] > 122)) {



				if (nameOrFunc) {
					if (tempStr != "") {
						for (int var = 0; var < systemData.varNames.size(); var++) {
							if (tempStr == systemData.varNames[var]) {
								if (var == eqNum && inFunc) {
									*simpleIterations = true; return result;
								}
							}
						}
						for (int par = 0; par < systemData.parameters.size(); par++) {
							if (tempStr == systemData.parameters[par]) {

							}
						}
					}
					if (!varOrParFound && original[i] == '(') {
						perenthesisCount--;
						inFunc = true;
					}
					else varOrParFound = false;
					nameOrFunc = false;
					tempStr = "";
				}
				if (original[i] == ')')perenthesisCount++;
				else if (original[i] == '(')perenthesisCount--;
				
				if (perenthesisCount == 0 && inFunc)inFunc = false;
			}
			else if (original[i] >= 48 && original[i] <= 57 && nameOrFunc || (original[i] >= 65 && original[i] <= 90) || (original[i] >= 97 && original[i] <= 122)) {
				nameOrFunc = true;
				tempStr += original[i];

			}
		}
		//		THIS DOES NOT TAKE ACOUNT FOR POWERS OF THE VARIABLE DONE AS IN "X * X" OR  THINGS SUCH AS "1 / X"   /// NEED TO ADD CHECK
	}

	return result;
}