//Graph format: 
//Simplified json format: 
//src degree dest0 dest1 ...

#include "comm.h"
#include "graph.h"

#define FILE_NOT_EXIST	1
#define FILE_EXIST	0
using namespace std;

graph::graph(
	string jsonfile)//,
{
	cout<<"read from folder "<<jsonfile<<endl;
	
	string s_begin = jsonfile+"/begin.bin";
	string s_adj = jsonfile+"/adjacent.bin";
	string s_head = jsonfile+"/head.bin";
	string s_degree = jsonfile+"/degree.bin";

	char* begin_file = const_cast<char*>(s_begin.c_str());
	char* adj_file = const_cast<char*>(s_adj.c_str());
	char* head_file = const_cast<char*>(s_head.c_str());
	char* degree_file = const_cast<char*>(s_degree.c_str());


	FILE *pFile= fopen(adj_file,"rb");
	adj_list = (vertex_t *)malloc(fsize(adj_file));
	fread(adj_list,sizeof(vertex_t),edge_count,pFile);
	fclose(pFile);

	FILE *pFile1= fopen(head_file,"rb");
	head_list = (vertex_t *)malloc(fsize(head_file));
	fread(head_list,sizeof(vertex_t),edge_count,pFile1);
	fclose(pFile1);


	FILE *pFile3 = fopen(begin_file,"rb");
	beg_pos = (index_t *)malloc(fsize(begin_file));
	fread(beg_pos,sizeof(index_t),vert_count+1,pFile3);
	fclose(pFile3);

	count = (index_t *)malloc(THDS_NUM*BLKS_NUM*sizeof(index_t));

	vert_count = fsize(begin_file)/sizeof(index_t) - 1;
	edge_count = fsize(head_file)/sizeof(vertex_t);

	cout<<"vert:" << vert_count<<"  edge: "<<edge_count<<endl;
}

void quickSort(vertex_t * arr, index_t left, index_t right)
{
      index_t i = left, j = right;
      vertex_t tmp;
      vertex_t pivot = arr[(left + right) / 2];
 
      /* partition */
      while (i <= j) {
            while (arr[i] < pivot)
                  i++;
            while (arr[j] > pivot)
                  j--;
            if (i <= j) {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++;
                  j--;
            }
      };
 
      /* recursion */
      if (left < j)
            quickSort(arr, left, j);
      if (i < right)
            quickSort(arr, i, right);
}


void graph::sort_ranked_adj(){
	// sort the adj_list
	for (vertex_t i; i < vertex_count; i++){
		int begin = RankedBegin[i];
		int end = RankedBegin[i+1]-1;
		quickSort(RankedAdj, begin, end);
	}
}


void graph::rank_by_degree(){
	RankedBegin = new index_t [vertex_count+1];
	RankedBegin[0] = 0;

	#pragma omp parallel for num_threads(56) schedule(dynamic,1024)
	for (vertex_t i = 0; i < vertex_count; i++){
		RankedBegin[i+1] = 0;
		//Calculte the degree of head vertex
		vertex_t h = head_list[i];
		int dh = begin_pos[h+1]-begin_pos[h];
		for (index_t j = begin_pos[i]; j < begin_pos[i+1]; j++){
			//Calculte the degree of adj vertex
			index_t a = adj_list[j];
			int da = begin_pos[j+1] - begin_pos[j];
			if(dh<da || (dh==da && h<a)) RankedBegin[i+1]++;
		}
	}

	// The ranked begining position
	for (int i = 0; i < vertex_count; i++){
		RankedBegin[i+1] += RankedBegin[i];
	}

	RankedEdgeCount = RankedBegin[vertex_count];
	RankedHead = new index_t[vertex_count];
	RankedAdj = new index_t[vertex_count];

	#pragma omp parallel for num_threads(56) schedule(dynamic,1024)
	for (vertex_t i = 0; i < vertex_count; i++){
		vertex_t h = head_list[i];
		int dh = begin_pos[h+1]-begin_pos[h];
		int k = RankedBegin[i];

		for (index_t j = begin_pos[i]; j < begin_pos[i+1]; j++){
			RankedHead[k] = head_list[j]
			RankedAdj[k] = adj_list[j];
			k++;
		}
	}

}


graph::~graph(){
	delete[] adj_list;
	delete[] head_list;
	delete[] beg_pos;
	delete[] RankedAdj;
	delete[] RankedHead;
	delete[] RankedBegin;
	delete[] RankedDegree;
}

