#include "pushrelabel.h"

#include "cuda.h"
#include <cstdio>
#include <cmath>
#include <iostream>
using namespace std;

#define TOTAL 32
#define BLOCK_DIM_X 32

int pushrelabel(int N, int s_x, int s_y, int t_x, int t_y, int* capacity){
	cuInit(0);
    CUdevice cuDevice;
    CUresult res = cuDeviceGet(&cuDevice, 0);

    CUcontext cuContext;
    res = cuCtxCreate(&cuContext, 0, cuDevice);

    CUmodule cuModule = (CUmodule)0;
    res = cuModuleLoad(&cuModule, "pushrelabel.ptx");

    CUfunction pushrelabelGPU;
    res = cuModuleGetFunction(&pushrelabelGPU, cuModule, "testOnGPU");

    int *excess, *height;
	excess = (int*) malloc(sizeof(int)*N);
	height = (int*) malloc(sizeof(int)*N);
	for(int i = 0; i<N; i++) excess[i] = height[i] = 0;


    CUdeviceptr capacityGPU;
    CUdeviceptr heightGPU;
    CUdeviceptr excessGPU;

    res = cuMemAlloc(&capacityGPU, 4*N*N*sizeof(int));
    res = cuMemAlloc(&heightGPU, N*sizeof(int));
    res = cuMemAlloc(&excessGPU, N*sizeof(int));
    cuCtxSynchronize();

    res = cuMemcpyHtoD(capacityGPU, (void*)capacity, 4*N*N*sizeof(int));
    res = cuMemcpyHtoD(excessGPU, (void*)excess, N*sizeof(int));
    res = cuMemcpyHtoD(heightGPU, (void*)height, N*sizeof(int));
    cuCtxSynchronize();


    void* args[] = {&capacityGPU, &excessGPU, &heightGPU, &N, &s_x, &s_y, &t_x, &t_y};

    res = cuLaunchKernel(pushrelabelGPU, TOTAL/BLOCK_DIM_X, 1, 1, BLOCK_DIM_X, 1, 1, 0, 0, args, 0);
    cuCtxSynchronize();

    // res = cuMemcpyDtoH( (void*)result, gpu_output, size*sizeof(int));
    
    // cuCtxSynchronize();
    // cuMemHostUnregister(result);


    // cuMemFree(gpu_output);
    // cuMemFree(gpu_input);
    
    // cuCtxSynchronize();
    // for(int i = 0; i<size; i++){
    // 	//result[i] = array[size-i-1];
    // }
    return 0; 
}