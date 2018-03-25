//Graph format: Json based format
//Storage format: 
//struct{
//		int: src_ver
//		Arr: [ver_0|ver_1|ver_2|...]
//		Int: num_conn_ver
//	}

#include "graph.h"
#include "graph.cpp"
#include "wtime.h"

#include <sstream>
#include <iostream>
#include <fstream>

#define N 256*256

using namespace std;

int main(int args, char *argv[]) {
	std::cout<<"Input format: ./exe graph-file-name"
						<<" (json formated file)\n";

	if(args != 2) return -1;
	graph *graph_d = new graph(argv[1]); 

	double t0=wtime();
	graph_d->rank_by_degree();
	double t1=wtime();
	cout<<"pre-processing time = "<<t1-t0<<" secondes"<<endl;

	double time=0;
	t0=wtime();
	graph_d->validation();
	t1=wtime();
	cout<<"total time = "<<t1-t0<<" secondes"<<endl;
	time += t1-t0;

	return 0;
}
