//Graph format: Json based format
//Storage format: 
//struct{
//		int: src_ver
//		Arr: [ver_0|ver_1|ver_2|...]
//		Int: num_conn_ver
//	}
/* main.cu */
#include "graph.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <pthread.h>
#define N 256*256
using namespace std;



int main(int args, char *argv[]) {
//	pthread_t thd1;
	std::cout<<"Input format: ./exe graph-file-name"
						<<" (json formated file)\n";

	if(args != 2) return -1;
	string json_file 	= argv[1];
	graph *graph_d 
		= new graph	(json_file); 
	
	cout<<"GPU  NUMBER = "<<GPU_NUM<<endl;
	cout<<"PART NUMBER = "<<PART_NUM<<endl;

//	mygraph=graph_d;
	cout<<"rank by degree\n";
double tt0=wtime();
	graph_d->preproc();
double tt1=wtime();
cout<<"pre-processing time = "<<tt1-tt0<<endl;

	double total_t=0;
int r=2;
for(int n=0; n<r; n++){
	graph_d->copy_time = 0;
	double t0=wtime();
//	index_t total = 0;

//int tid;
omp_set_nested(1);
{
//#pragma omp parallel for num_threads(GPU_NUM+1)
#pragma omp parallel for num_threads(GPU_NUM)
//	for(int i=0; i<GPU_NUM+1; i++){
	for(int i=0; i<GPU_NUM; i++){
//		tid = omp_get_thread_num();
//		if(i<GPU_NUM){
			graph_d->gpuProc(i);
//		}
//		else if(i == GPU_NUM){
//			graph_d->cpuProc();
//		}
//		else cout<<"tid = "<<i<<endl;
	}
}

	
//	for(int i=0; i<GPU_NUM+1; i++){
//		total+= graph_d->count[i];
//	}
	graph_d->reduceResult();
	double t1=wtime();
	cout<<"total count "<<graph_d->count[0]<<"\n";
	cout<<"total time  "<<t1-t0<<" seconds\n";
	total_t += t1-t0;
	cout<<"copy time  "<<graph_d->copy_time<<" seconds\n";
}
cout<<"merge average time of 5 round = "<<total_t/r<<endl;
	return 0;
}
