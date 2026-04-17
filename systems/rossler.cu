#include "rossler.h "
#define name rossler

namespace attributes
{
enum variables { x, y, z };
enum parameters { a, b, c, method, COUNT };
enum methods { ExplicitEuler, ExplicitMidpoint, ExplicitRungeKutta4 };
}

__global__ void gpu_wrapper_(name)(Computation* data, uint64_t variation)
{
    kernelProgram_(name)(data, (blockIdx.x* blockDim.x) + threadIdx.x);
}
__host__ __device__ void kernelProgram_(name)(Computation* data, uint64_t variation)
{
    if (variation >= CUDA_marshal.totalVariations) return;   // Shutdown thread if there isn't a variation to compute
    uint64_t stepStart, variationStart = variation * CUDA_marshal.variationSize;         // Start index to store the modelling data for the variation
    LOCAL_BUFFERS;
    LOAD_ATTRIBUTES(false);
    // Custom area (usually) starts here
    TRANSIENT_SKIP_NEW(finiteDifferenceScheme_(name));
    for (int s = 0; s < CUDA_kernel.steps && !data->isHires; s++)
    {
        stepStart = variationStart + s * CUDA_kernel.VAR_COUNT;
        finiteDifferenceScheme_(name)(FDS_ARGUMENTS);
        RECORD_STEP;
    }

    // Analysis
    AnalysisLobby(data, &finiteDifferenceScheme_(name), variation);
}
__host__ __device__ __forceinline__ void finiteDifferenceScheme_(name)(numb* currentV, numb* nextV, numb* parameters, PerThread* pt)
{

    ifMETHOD(P(method), ExplicitEuler)
   {
        Vnext(x) = V(x) + H * (- V(y) - V(z));
        Vnext(y) = V(y) + H * (V(x) + P(a) * V(y));
        Vnext(z) = V(z) + H * (P(b) + V(z) * (V(x) - P(c)));
    }

    ifMETHOD(P(method), ExplicitMidpoint)
   {
        numb xmp = V(x) + (numb)0.5 * H * (- V(y) - V(z));
        numb ymp = V(y) + (numb)0.5 * H * (V(x) + P(a) * V(y));
        numb zmp = V(z) + (numb)0.5 * H * (P(b) + V(z) * (V(x) - P(c)));
        numb xmp = V(x) + H * (- ymp - zmp);
        numb ymp = V(y) + H * (xmp + P(a) * ymp);
        numb zmp = V(z) + H * (P(b) + zmp * (xmp - P(c)));
    }

}
