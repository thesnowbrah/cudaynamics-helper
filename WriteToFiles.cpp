#include "WriteToFiles.h"

void WriteTXT(systemStruct systemData) {
	std::ofstream OutputTXT("systems/" + systemData.systemNameCode + "/" + systemData.systemNameCode + ".txt");
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
	OutputTXT << "param symmetry Fixed 0.0 1.0 0.01 100 0.0 0.0\n";

	OutputTXT << "// Defining enumerated parameters (useful for methods):\n"
		<< "// enum <name> <ranging type> <minimum value> <maximum value> <step> <step count> <normal mean> <normal deviation> <enum names, no spaces>\n";
		
	
	if (hasSignal)OutputTXT << "enum signal 1square 0sine 0triangle\n";

	OutputTXT<< "enum method 1ExplicitEuler 0SemiExplicitEuler 0ExplicitMidpoint 0ExplicitRungeKutta4 0VariableSymmetryCD\n"
		<< "// Defining settings for analysis functions:\n"
		<< "// analysis <name from \"anfunc_names.cpp\"> settings <values, must exactly match the settings struct> \n"
		<< "analysis Minimum / maximum settings 2 2\n"
		<< "analysis Largest Lyapunov exponent settings 0.01 30 0 0 1 2 -1\n";

	OutputTXT.close();

}

void WriteHFile(systemStruct systemData) {
	std::ofstream OutputTXT("systems/" + systemData.systemNameCode + "/" + systemData.systemNameCode + ".h");
	OutputTXT << "#pragma once\n#include <kernels_common.h>\n\n"
		<< "#define name " << systemData.systemNameCode << "\n\n"
		<< "const int THREADS_PER_BLOCK_(name) = 64;\n\n"
		<< "__global__ void gpu_wrapper_(name)(Computation* data, uint64_t variation);\n\n"
		<< "__host__ __device__ void kernelProgram_(name)(Computation* data, uint64_t variation);\n\n"
		<< "__host__ __device__ __forceinline__ void finiteDifferenceScheme_(name)(numb* currentV, numb* nextV, numb* parameters, PerThread* pt);\n\n"
		<< "#undef name\n\n";

	OutputTXT.close();
}

void WriteCuFile(systemStruct systemData) {
	std::ofstream OutputTXT("systems/" + systemData.systemNameCode + "/" + systemData.systemNameCode + ".cu");
	OutputTXT << "#include \"" + systemData.systemNameCode + ".h \"\n";
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
	OutputTXT << "symmetry, method, COUNT };\n";

	bool hasSignal = false;

	for (int i = 0; i < systemData.varNames.size(); i++) {
		if (systemData.varEqs[i] == "signal") {
			hasSignal = true;
		}
	}

	if (hasSignal)OutputTXT << "enum waveforms { square, sine, triangle };\n";

	OutputTXT << "enum methods { ";
	OutputTXT << "ExplicitEuler,  SemiExplicitEuler, ExplicitMidpoint, ExplicitRungeKutta4, VariableSymmetryCD};\n}\n\n";

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


	int temp;
	if (hasSignal)temp = 3;
	else temp = 1;

	for (int signal = 0; signal < temp; signal++) {
		
		if (hasSignal && signal == 0) OutputTXT << "    ifSIGNAL(P(signal), square)\n    {\n";
		else if(signal == 1)OutputTXT << "    ifSIGNAL(P(signal), sine)\n    {\n";
		else if (signal == 2)OutputTXT << "    ifSIGNAL(P(signal), triangle)\n    {\n";

		//			EXPLICIT EULER
		OutputTXT << "    ifMETHOD(P(method), ExplicitEuler)\n    {\n";
				
		if(hasSignal)
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
				OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H * (" << ChangeEqsToKernelExplicitEuler(systemData, systemData.varEqs[i]) << ");\n";
			}
		}

		OutputTXT << "    }\n\n";
		//			EXPLICIT EULER

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

			else if (systemData.varEqs[i] != "signal"){
				OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H * (" << ChangeEqsToKernelSemiExplicit(systemData, systemData.varEqs[i], i) << ");\n";
			}
		}

		OutputTXT << "    }\n\n";
		//			EXPLICIT EULER-CROMER

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

			else if (systemData.varEqs[i] != "signal"){
				OutputTXT << "        numb " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + (numb)0.5 * H * (" << ChangeEqsToKernelExplicitEuler(systemData, systemData.varEqs[i]) << ");\n";
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

			else if(systemData.varEqs[i] != "signal") {
				OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ")" << " + H * (" << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ");\n";
			}
		}

		OutputTXT << "    }\n\n";
		//			EXPLICIT MIDPOINT



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

			else if (systemData.varEqs[i] != "signal"){
				OutputTXT << "        numb k" << systemData.varNames[i] << "1 = " << ChangeEqsToKernelExplicitEuler(systemData, systemData.varEqs[i]) << ";\n";
			}
		}
		OutputTXT << "\n";
		for (int i = 0; i < systemData.varNames.size(); i++) {
			if(systemData.varEqs[i] != "signal")
			OutputTXT << "        numb " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + (numb)0.5 * H * k" << systemData.varNames[i] << "1;\n";
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

			else if(systemData.varEqs[i] != "signal"){
				OutputTXT << "        numb k" << systemData.varNames[i] << "2 = " << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ";\n";
			}
		}
		OutputTXT << "\n";
		for (int i = 0; i < systemData.varNames.size(); i++) {
			if (systemData.varEqs[i] != "signal")
			OutputTXT << "        " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + (numb)0.5 * H * k" << systemData.varNames[i] << "2;\n";
		}
		OutputTXT << "\n";

		for (int i = 0; i < systemData.varNames.size(); i++) {
			if(systemData.varEqs[i] != "Time" && systemData.varEqs[i] != "signal") OutputTXT << "        numb k" << systemData.varNames[i] << "3 = " << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ";\n";
			else if(systemData.varEqs[i] == "Time") OutputTXT << "        numb k" << systemData.varNames[i] << "3 = 1" << ";\n";

		}
		OutputTXT << "\n";
		for (int i = 0; i < systemData.varNames.size(); i++) {
			if (systemData.varEqs[i] != "signal")
			OutputTXT << "        " << systemData.varNames[i] << "mp = V(" << systemData.varNames[i] << ")" << " + H * k" << systemData.varNames[i] << "3;\n";
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
			else if (systemData.varEqs[i] != "signal"){
				OutputTXT << "        numb k" << systemData.varNames[i] << "4 = " << ChangeEqsToKernelExplicitMidpoint(systemData, systemData.varEqs[i]) << ";\n";
			}
		}
		OutputTXT << "\n";

		for (int i = 0; i < systemData.varNames.size(); i++) {
			if(systemData.varEqs[i] != "signal")
			OutputTXT << "        Vnext(" << systemData.varNames[i] << ") = V(" << systemData.varNames[i] << ") + H * (k" << systemData.varNames[i] << "1"
				<< " + (numb)2.0 * k" << systemData.varNames[i] << "2" << " + (numb)2.0 * k" << systemData.varNames[i] << "3"
				<< " + k" << systemData.varNames[i] << "4) / (numb)6.0" << ";\n";
		}

		OutputTXT << "    }\n\n";
		//			EXPLICIT RK4

		//			EXPLICIT VSCD
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
		//			EXPLICIT VSCD

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
								result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
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
							result += "V("; result += tempStr; result += ")"; varOrParFound = true; break;
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
							result += tempStr; result += "mp"; varOrParFound = true; break;
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
						result += tempStr; result += "mp"; varOrParFound = true; break;
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