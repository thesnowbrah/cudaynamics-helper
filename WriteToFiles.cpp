#include "WriteToFiles.h"

void WriteTXT(systemStruct systemData) {
	std::ofstream OutputTXT(systemData.systemNameCode +".txt");
	OutputTXT << "Name: " << systemData.systemNameTXT << "\n" << "Steps: 10000\n"<< "Transient: 10000\n";
	OutputTXT << "// Defining step: \n"
		<< "// parameter/variable/discrete\n"
		<< "// Then, unless it is discrete, provide its name and define it like an attribute\n"
		<< "// If it is discrete, add nothing after\n";
	OutputTXT << "Step type: parameter h Fixed 0.01 0.1 0.01 100 0.0 0.0\n";
	OutputTXT << "// Compute on CUDAynamics launch: yes/no\n"
		<< "Execute on launch : yes\n"
		<< "// Ranging types:\n"
		<< "// Fixed (single <minimum value>)\n"
		<< "// Linear (<step count> values uniformly distributed between <minimum value> and <maximum value>, inclusively)\n"
		<< "// Step (values are picked from <minimum value> to <maximum value> with a step <step>, including minimum, including maximum if it ends up as a picked value)\n"
		<< "// Random (<step count> values randomly picked from <minimum value> to <maximum value>)\n"
		<< "// Normal (normal distribution of <step count> values, defined by <normal mean> and <normal deviation>\n"
		<< "//\n"
		<< "// Defining variables/parameters:\n"
		<< "// var/param <name> <ranging type> <minimum value> <maximum value> <step> <step count> <normal mean> <normal deviation>\n";
	for (int i = 0; i < systemData.varNames.size(); i++) {
		OutputTXT << "var" << systemData.varNames[i] << "Fixed 0.0 10.0 1.0 50 0.0 0.0\n";
	}
	for (int i = 0; i < systemData.parameters.size(); i++) {
		OutputTXT << "param" << systemData.parameters[i] << "Fixed 0.0 10.0 1.0 50 0.0 0.0\n";
	}
	//when VSCD is ready add symmetry param

	OutputTXT << "// Defining enumerated parameters (useful for methods):\n"
		<< "// enum <name> <ranging type> <minimum value> <maximum value> <step> <step count> <normal mean> <normal deviation> <enum names, no spaces>\n"
		<< "enum method 0ExplicitEuler 0ExplicitMidpoint 0ExplicitRungeKutta4 1VariableSymmetryCD\n"
		<< "//\n"
		<< "// Defining settings for analysis functions:\n"
		<< "// analysis <name from \"anfunc_names.cpp\"> settings <values, must exactly match the settings struct> \n"
		<< "analysis Minimum / maximum settings 2 2\n"
		<< "analysis Largest Lyapunov exponent settings 0.01 30 0 0 1 2 - 1\n";

	OutputTXT.close();

}

void WriteHFile(systemStruct systemData) {
	std::ofstream OutputTXT(systemData.systemNameCode + ".h");
	OutputTXT << "#pragma once\n#include <kernels_common.h>\n\n"
		<< "#define name" << systemData.systemNameCode << "\n\n"
		<< "const int THREADS_PER_BLOCK_(name) = 64;\n\n"
		<< "__global__ void gpu_wrapper_(name)(Computation* data, uint64_t variation);\n\n"
		<< "__host__ __device__ void kernelProgram_(name)(Computation* data, uint64_t variation);\n\n"
		<< "__host__ __device__ __forceinline__ void finiteDifferenceScheme_(name)(numb* currentV, numb* nextV, numb* parameters);\n\n"
		<< "#undef name\n\n";

	OutputTXT.close();
}

void WriteCPPFile(systemStruct systemData) {
	std::ofstream OutputTXT(systemData.systemNameCode + ".cpp");
	OutputTXT << "#include \"" + systemData.systemNameCode + ".h \"\n";
	OutputTXT << "#define name" << systemData.systemNameCode << "\n\n";

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
	OutputTXT << "method, COUNT };\n";

	OutputTXT << "enum methods { ";
	OutputTXT << "ExplicitEuler, ExplicitMidpoint, ExplicitRungeKutta4 };\n}\n\n";//, VariableSymmetryCD

	OutputTXT << "__global__ void gpu_wrapper_(name)(Computation* data, uint64_t variation)\n"
		<< "{\nkernelProgram_(name)(data, (blockIdx.x* blockDim.x) + threadIdx.x);\n}\n";

	OutputTXT << "__host__ __device__ void kernelProgram_(name)(Computation* data, uint64_t variation)\n{\n"
		<< "if (variation >= CUDA_marshal.totalVariations) return;   // Shutdown thread if there isn't a variation to compute\n"
		<< "uint64_t stepStart, variationStart = variation * CUDA_marshal.variationSize;         // Start index to store the modelling data for the variation\n"
		<< "LOCAL_BUFFERS;\nLOAD_ATTRIBUTES(false);\n"
		<< "// Custom area (usually) starts here\n"
		<< "TRANSIENT_SKIP_NEW(finiteDifferenceScheme_(name));\n"
		<< "for (int s = 0; s < CUDA_kernel.steps && !data->isHires; s++)\n{\n"
		<< "stepStart = variationStart + s * CUDA_kernel.VAR_COUNT;\n"
		<< "finiteDifferenceScheme_(name)(FDS_ARGUMENTS);\nRECORD_STEP;\n}\n\n"
		<< "// Analysis\nAnalysisLobby(data, &finiteDifferenceScheme_(name), variation);\n}\n";
		
	OutputTXT << "__host__ __device__ __forceinline__ void finiteDifferenceScheme_(name)(numb* currentV, numb* nextV, numb* parameters)\n{\n"
		<< "ifMETHOD(P(method), ExplicitEuler)\n{\n";
	for (int i = 0; i < systemData.varNames.size(); i++) {
		OutputTXT << "Vnext(" << systemData.varNames[i] << ") = ";
		///////OUTPUT OF EQUATIONS WITH V(X) AND P(X) REPLACED FIRST
	}

	OutputTXT << "}\n";

}