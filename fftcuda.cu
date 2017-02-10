#include "cufft.h"
#include "cuda_runtime_api.h"

typedef float2 Complex;

void
PerformCUDAFFT(float *inputData, float *outputData, unsigned int numSamples) {
    cufftHandle plan;
    cufftComplex *inputDataG, *outputDataG;
    int i;

    float *inputDataC, *outputDataC;
    outputDataC = (float*) malloc(sizeof(float) * numSamples * 2);
    inputDataC  = (float*) malloc(sizeof(float) * numSamples * 2);

    for (i = 0; i < numSamples; i++) {
      inputDataC[i*2]     = inputData[i];
      inputDataC[i*2 + 1] = 0.0f;
    }

    cudaMalloc((void**)&inputDataG,  sizeof(cufftComplex)*numSamples);
    cudaMalloc((void**)&outputDataG, sizeof(cufftComplex)*numSamples);

    cudaMemcpy(inputDataG, inputDataC, sizeof(cufftComplex)*numSamples, cudaMemcpyHostToDevice);
    cufftPlan1d(&plan, numSamples, CUFFT_C2C, 1);
    cufftExecC2C(plan, inputDataG, outputDataG, CUFFT_FORWARD);
    cufftDestroy(plan);
    cudaMemcpy(outputDataC, outputDataG, sizeof(cufftComplex)*numSamples, cudaMemcpyDeviceToHost);
    cudaFree(inputDataG);
    cudaFree(outputDataG);
    
    for (i = 0; i < numSamples; i++) {
        outputData[i] = outputDataC[i * 2];
    }
    free(outputDataC);
    free(inputDataC);
}
