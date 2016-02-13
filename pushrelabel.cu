extern "C" {

__global__ void testOnGPU(int* capacityGPU, int* excessGPU, int* heightGPU, int N, int s_x, int s_y, int t_x, int t_y)
{
	printf("Hello\n");
}

};
