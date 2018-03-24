#include <iostream>

#include "comm.h"
#include "graph.h"
#include "wtime.h"

using namespace std;

int main(int argc, char* argv[]){
	double t0, t1, total_time = 0;
	const int round = 3;

	if(argc != 2){
		perror("Please run the code with one peremeter, which is the directary path of dataset");
		return -1;
	}
	// ? G must be graph*, cannot be graph, or when cudaSetDevice, it will corrupt
	graph* G = new graph(argv[1]);

	//Rank by degree
	t0 = wtime();
	G->rank_by_degree();
	t1 = wtime();
	cout << "Time of rank by degree: " << t1 - t0 << " s" << endl;

	//Sort ranked adj_list
	t0 = wtime();
	G->sort_ranked_adj();
	t1 = wtime();
	cout << "Time of sort ranked adj: " << t1 - t0 << " s" << endl;

	//Calculate triangles round times to get avarage time.
	for (int i = 0; i < round; i++){
		t0 = wtime();
		G->scan();
		t1 = wtime();
		cout << "Round " << i+1 << ": Time of scan: " << t1 - t0 << "s" << endl;
		total_time += t1-t0;
	}

	cout << "Avarage time of " << round << " round(s) :" <<  total_time/round << endl;

	return 0;
}
