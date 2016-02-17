#define INF 2000000000
#define GID(x, y) ( ((y)*N) + (x))

extern "C" {
__global__ void testOnGPU(int* capacityGPU, int* excessGPU, int* heightGPU, 
	int N, int s_x, int s_y, int t_x, int t_y, int* changed, int *mapGPU)
{
	// *changed = 0;
	// int blockId = blockIdx.x + blockIdx.y * gridDim.x; 
	//int u = blockId * (blockDim.x * blockDim.y) + (threadIdx.y * blockDim.x) + threadIdx.x;
	
	int x = threadIdx.x + blockIdx.x*blockDim.x;
    int y = threadIdx.y + blockIdx.y*blockDim.y;
    int u = GID(x, y);
    // testing macro and coordinatce calculations
    // mapGPU[u] = x*100 + y;
    // mapGPU[u] = GID(x, y);
    // mapGPU[u] = ( u==GID(x, y) ? 1 : 0 );
    // return;



	// exit if source or sink
	if( (GID(s_x, s_y) == u) || (GID(t_x, t_y) == u)){
		return;
	}

	if(excessGPU[u] > 0){
		int e_temp = excessGPU[u];
		int h_temp = INF;
		int v_temp = -1;
		// int v_tempY = -1;
		int u_to_v = -1; 
		int v_to_u = -1;

		/* find lowest neighboor v_temp of u */

		// capacityGPU[4*ID+0] up
		// capacityGPU[4*ID+1] right
		// capacityGPU[4*ID+2] down
		// capacityGPU[4*ID+3] left

		// EDGE UP
		if ( capacityGPU[4 * u + 0] > 0 ){
			int v = GID(x, y-1);
			if (heightGPU[v] < h_temp) {
				u_to_v = 0;
				v_to_u = 2;
				v_temp = v;
				h_temp = heightGPU[v];
			}
		}
		// EDGE DOWN
		if ( capacityGPU[4 * u + 2] > 0 ){
			int v = GID(x, y+1);
			if (heightGPU[v] < h_temp) {
				u_to_v = 2;
				v_to_u = 0;
				v_temp = v;
				h_temp = heightGPU[v];
			}
		}
		//EDGE LEFT
		if ( capacityGPU[4 * u + 1] > 0 ){
			int v = GID(x+1, y);
			if (heightGPU[v] < h_temp) {
				u_to_v = 1;
				v_to_u = 3;
				v_temp = v;
				h_temp = heightGPU[v];
			}
		}
		// EDGE RIGHT
		if ( capacityGPU[4 * u + 3] > 0 ){
			int v = GID(x-1, y);
			if (heightGPU[v] < h_temp) {
				u_to_v = 3;
				v_to_u = 1;
				v_temp = v;
				h_temp = heightGPU[v];
			}
		}
		// //EDGE UP
		// if(y > 0){
		// 	int v = GID(x, y-1);
		// 	if(heightGPU[v] < h_temp && capacityGPU[4*u+0] > 0){
		// 		v_temp = GID(x, y-1);
		// 		// v_tempY = y-1;
		// 		u_to_v = 0;
		// 		v_to_u = 2;
		// 		h_temp = heightGPU[v];
		// 	}
		// }

		// //EDGE DOWN
		// if(y < N-1){
		// 	int v = GID(x, y+1);
		// 	if(heightGPU[v] < h_temp && capacityGPU[4*u+2] > 0){
		// 		v_temp = GID(x, y+1);
		// 		// v_tempY = y+1;
		// 		u_to_v = 2;
		// 		v_to_u = 0;
		// 		h_temp = heightGPU[v];
		// 	}
		// }

		// //EDGE LEFT
		// if(x > 0){
		// 	int v = GID(x-1, y);
		// 	if(heightGPU[v] < h_temp && capacityGPU[4*u+1] > 0){
		// 		v_temp = GID(x-1, y);
		// 		// v_tempY = y;
		// 		u_to_v = 1;
		// 		v_to_u = 3;
		// 		h_temp = heightGPU[v];
		// 	}
		// }

		// //EDGE RIGHT
		// if(x < N-1){
		// 	int v = GID(x+1, y);
		// 	if(heightGPU[v] < h_temp && capacityGPU[4*u+3] > 0){
		// 		v_temp = GID(x+1, y);
		// 		// v_tempY = y;
		// 		u_to_v = 3;
		// 		v_to_u = 1;
		// 		h_temp = heightGPU[v];
		// 	}
		// }

		if(v_temp == -1 ){
			printf("Something is wrong: %d %d\n", x, y);
			return;
		}

		if(heightGPU[u] > h_temp){
			int d = min(e_temp, capacityGPU[4*u + u_to_v]);
			atomicSub(&capacityGPU[4*u+u_to_v], d);			//edge from u to v_temp
			atomicAdd(&capacityGPU[4*v_temp + v_to_u], d);	//edge from v_temp to u
			
			atomicSub(&excessGPU[u], d);
			atomicAdd(&excessGPU[v_temp], d);

			
		} else {
			heightGPU[u] = h_temp + 1;
		}
		*changed = 1;
		//mapGPU[u] = capacityGPU[u];
	}
}

};
