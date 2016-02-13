#include "pushrelabel.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
using namespace std;

int main() {
    int N = 32;
    int* capacity = (int*) malloc(sizeof(int)*4*N*N);
    int zrodlo_x = 5;
    int zrodlo_y = 5;
    int ujscie_x = 0;
    int ujscie_y = 0;
    for (int i = 0; i < 4*N*N; ++i){
        capacity[i] = 0;
    }
    for (int y = 0; y < N; ++y){
        for(int x = 0; x < N; ++x){
            if (x+1 < N){
                //(x, y) -> (x+1, y) oraz (x+1, y) -> (x, y)
                capacity[4*(N*y+x)+1] = capacity[4*(N*y+x+1)+3] = 10;
            }
            if (y+1 < N){
                //(x, y) -> (x, y+1) oraz (x, y+1) -> (x, y)
                capacity[4*(N*y+x)+2] = capacity[4*(N*(y+1)+x)] = 10;
            }
        }
    }
    //kasowanie krawedzi wychodzacych z ujscia
    //up
    capacity[4*(N*ujscie_y+ujscie_x)] = 0;
    //left
    capacity[4*(N*ujscie_y+ujscie_x)+1] = 0;
    //down
    capacity[4*(N*ujscie_y+ujscie_x)+2] = 0;
    //right
    capacity[4*(N*ujscie_y+ujscie_x)+3] = 0;
 
    int res = pushrelabel(N, zrodlo_x, zrodlo_y, ujscie_x, ujscie_y, capacity);
    printf("%d\n", res);
    free(capacity);
    return 0;
}
