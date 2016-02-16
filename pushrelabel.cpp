#include "pushrelabel.h"

#include "cuda.h"
#include <cstdio>
#include <cmath>
#include <iostream>
#include <cstring>

using namespace std;

#define BLOCK_DIM_X 32
#define BLOCK_DIM_Y 32

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define GID(x, y) ( ((y)*N) + (x))

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


    int *excess, *height, *cf, *map;
    volatile int changed = 1234567;

	excess = (int*) malloc(N*N*sizeof(int));
	height = (int*) malloc(N*N*sizeof(int));
    map = (int*) malloc(N*N*sizeof(int));

    cf = (int*) malloc(4*N*N*sizeof(int));
    

	for(int i = 0; i<N*N; i++){
        excess[i] = map[i] = 0;
    }

    /* h(s) = |V| */
    height[GID(s_x, s_y)] = N*N;

    /* Init all h(u) with 0 except for u == source */
    for(int i = 0; i<N*N; i++){
        if(i != GID(s_x, s_y)){
            height[i] = 0;
        }
    }

    /* Init cf(u, v) for all edges */
    for(int i = 0; i<4*N*N; i++){
        cf[i] = capacity[i];
    }

    /* Init for everything going from source */
    
    // capacityGPU[4*ID+0] up
    // capacityGPU[4*ID+1] right
    // capacityGPU[4*ID+2] down
    // capacityGPU[4*ID+3] left
    
    //EDGE UP
    if(s_y > 0){
        int v = GID(s_x, s_y - 1);
        cf[4*GID(s_x, s_y) + 0] = 0;
        cf[4*v + 2] = capacity[4*v + 2] + capacity[4*GID(s_x, s_y) + 0];
        excess[v] = capacity[4*GID(s_x, s_y) + 0];
    }

    //EDGE DOWN
    if(s_y < N-1){
        int v = GID(s_x, s_y+1);
        cf[4*GID(s_x, s_y) + 2] = 0;
        cf[4*v + 0] = capacity[4*v + 0] + capacity[4*GID(s_x, s_y) + 2];
        excess[v] = capacity[4*GID(s_x, s_y) + 2];
    }

    //EDGE LEFT
    if(s_x > 0){
        int v = GID(s_x-1, s_y);
        cf[4*GID(s_x, s_y) + 3] = 0;
        cf[4*v + 1] = capacity[4*v + 1] + capacity[4*GID(s_x, s_y) + 3];
        excess[v] = capacity[4*GID(s_x, s_y) + 3];
    }

    //EDGE RIGHT
    if(s_x < N-1){
        int v = GID(s_x+1, s_y);
        cf[4*GID(s_x, s_y) + 1] = 0;
        cf[4*v + 3] = capacity[4*v + 3] + capacity[4*GID(s_x, s_y) + 1];
        excess[v] = capacity[4*GID(s_x, s_y) + 1];
    }

    CUdeviceptr capacityGPU;
    CUdeviceptr heightGPU;
    CUdeviceptr excessGPU;
    CUdeviceptr changedGPU;
    CUdeviceptr mapGPU;

    res = cuMemAlloc(&capacityGPU, 4*N*N*sizeof(int));
    res = cuMemAlloc(&heightGPU, N*N*sizeof(int));
    res = cuMemAlloc(&excessGPU, N*N*sizeof(int));
    res = cuMemAlloc(&mapGPU, N*N*sizeof(int));
    res = cuMemAlloc(&changedGPU, sizeof(int));
    cuCtxSynchronize();

    res = cuMemcpyHtoD(capacityGPU, (void*)cf, 4*N*N*sizeof(int));
    res = cuMemcpyHtoD(excessGPU, (void*)excess, N*N*sizeof(int));
    res = cuMemcpyHtoD(heightGPU, (void*)height, N*N*sizeof(int));
    res = cuMemcpyHtoD(mapGPU, (void*)map, N*N*sizeof(int));
    res = cuMemcpyHtoD(changedGPU, (void*)&changed, sizeof(int));
    cuCtxSynchronize();

    
    void* args[] = {&capacityGPU, &excessGPU, &heightGPU, &N, &s_x, &s_y, &t_x, &t_y, &changedGPU, &mapGPU};
    int counter = 0;
    while(changed && counter<= 150){
        cuCtxSynchronize();
    	changed = 0;
    	res = cuLaunchKernel(pushrelabelGPU, N/BLOCK_DIM_X, N/BLOCK_DIM_Y, 1, BLOCK_DIM_X, BLOCK_DIM_Y, 1, 0, 0, args, 0);
    	cuCtxSynchronize();
    	res = cuMemcpyDtoH( (void*)&changed, changedGPU, sizeof(int));  
        
        /*DEBUG*/ 	
        res = cuMemcpyDtoH( (void*) map, mapGPU, N*N*sizeof(int));  
        printf("%d %d\n", s_x, s_y);
        cout << "Kernel iteration: " << counter++ << endl;
    	printf("Changed in gpu %d\n", changed);
    	for(int i = 0; i<N; i++){
            for (int j = 0; j < N; j++){
                if( j == s_x && i == s_y){
                    printf(ANSI_COLOR_GREEN "%d " ANSI_COLOR_RESET, map[i*N+j]);
                } else {
                    map[i*N+j] == 0 ? printf("%d ", map[i*N+j]) : printf(ANSI_COLOR_RED "%d " ANSI_COLOR_RESET, map[i*N+j]);
                }
            }
            printf("\n");
        }
	}
    return 0; 
}