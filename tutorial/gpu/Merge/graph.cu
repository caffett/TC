#include <omp.h>
#include <stdio.h>

#include "comm.h"
#include "graph.h"
#include "wtime.h"

using namespace std;

/*******************GPU Functions***********************/
// 1 thread <-> 1 vertex -> imbalance
__global__ void vertex_merge_kernel(	
	vertex_t*	adj,
	index_t*	begin,
	index_t		Ns,	// The start thread id
	index_t		Ne,	// The end thread id
	index_t*	count
){
	__shared__ index_t local_count[THDS_NUM];
	index_t tid = Ns + (threadIdx.x + blockIdx.x * blockDim.x);
	index_t thd_count=0;

	while(tid<Ne){
		vertex_t A = tid;
		index_t degree_A = begin[A+1]-begin[A];
		vertex_t* a = &(adj[begin[A]]);

		for(int i=0; i<degree_A; i++){
			vertex_t B = adj[begin[A]+i];
			index_t degree_B = begin[B+1]-begin[B];
			vertex_t* b = &(adj[begin[B]]);

			index_t index_A  = 0;
			index_t index_B  = 0;
			index_t x,y;
			while(index_A<degree_A && index_B<degree_B){
				x=a[index_A];
				y=b[index_B];

				if(x<y){
					index_A++;
				}
				else if(x>y){
					index_B++;
				}
				else if(x==y){
					index_A++;
					index_B++;
					thd_count++;
				}
			}
		}
		tid += gridDim.x*blockDim.x;
	}
	//reduce
	local_count[threadIdx.x] = thd_count;
	__syncthreads();

	if(threadIdx.x==0){
		index_t val=0;
		for(int i=0; i<blockDim.x; i++){
			val += local_count[i];
		}
		count[blockIdx.x]=val;
	}
	__syncthreads();
}

__global__ void reduce_kernel(index_t* count)
{
	index_t val = 0;
	for(int i=0; i<BLKS_NUM; i++){
		val += count[i];
	}
	count[0] = val;
}

/*******************CPU Functions***********************/
void printGraph(vertex_t vertCount, 
		vertex_t* head, 
		vertex_t* adj, 
		index_t* begin){
	for(vertex_t i=0; i<vertCount; i++){
		if(begin[i+1]>begin[i]){
			cout<<begin[i]<<" "<<begin[i+1]-begin[i]<<": ";
		}
//		for(int j=0; j<degree[i]; j++){
		for(vertex_t j=0; j<begin[i+1]-begin[i]; j++){
			cout<<head[begin[i]+j]<<"-"<<adj[begin[i]+j]<<" ";
		}
		if(begin[i+1]>begin[i]){
			cout<<"\n";
		}
	}
}


void graph::scan(){
	// Select the GPU to excute kernel
	cudaSetDevice(1);

	// The variables of GPU
	index_t* dev_begin;
	vertex_t* dev_adj_list;
	index_t* dev_count;

	H_ERR(cudaMalloc((void**)&dev_begin, (vertex_count+1)*sizeof(index_t)));
	H_ERR(cudaMalloc((void**)&dev_adj_list, RankedEdgeCount*sizeof(vertex_t)));
	H_ERR(cudaMalloc((void**)&dev_count, BLKS_NUM*sizeof(index_t)));

	// Be careful, all the list have been ranked.
	H_ERR(cudaMemcpy(dev_adj_list, RankedAdj, RankedEdgeCount*sizeof(vertex_t), cudaMemcpyHostToDevice));
	H_ERR(cudaMemcpy(dev_begin, RankedBegin, (vertex_count+1)*sizeof(index_t), cudaMemcpyHostToDevice));

	double t0 = wtime();
	// call GPU merge based function kernel
	vertex_merge_kernel<<<BLKS_NUM,THDS_NUM>>>
	(	
		dev_adj_list,
		dev_begin,
		0,
		vertex_count,
		dev_count
	);
	H_ERR(cudaDeviceSynchronize());

	// reduce the results
	reduce_kernel <<<1,1>>>(dev_count);
	int result;
	H_ERR(cudaMemcpy(&result, dev_count, sizeof(int), cudaMemcpyDeviceToHost));
	double t1 = wtime();
	
	cout << "Total count: " << result << endl;
	cout << "Time consume: " << t1-t0 << " s" << endl;

	H_ERR(cudaFree(dev_count));
	H_ERR(cudaFree(dev_adj_list));
	H_ERR(cudaFree(dev_begin));
}


graph::graph(string jsonfile){
	cout<<"read from folder "<<jsonfile<<endl;
	
	string s_begin = jsonfile+"/begin.bin";
	string s_adj = jsonfile+"/adjacent.bin";
	string s_head = jsonfile+"/head.bin";
	string s_degree = jsonfile+"/degree.bin";

	char* begin_file = const_cast<char*>(s_begin.c_str());
	char* adj_file = const_cast<char*>(s_adj.c_str());
	char* head_file = const_cast<char*>(s_head.c_str());
	char* degree_file = const_cast<char*>(s_degree.c_str());

	vertex_count = fsize(begin_file)/sizeof(index_t) - 1;
	edge_count = fsize(head_file)/sizeof(vertex_t);

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
	fread(beg_pos,sizeof(index_t),vertex_count+1,pFile3);
	fclose(pFile3);

	count = (index_t *)malloc(THDS_NUM*BLKS_NUM*sizeof(index_t));

	cout<<"vert:" << vertex_count<<"  edge: "<<edge_count<<endl;
}

void quickSort(vertex_t* arr, index_t left, index_t right)
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
	for (vertex_t i = 0; i < vertex_count; i++){
		index_t begin = RankedBegin[i];
		index_t end = RankedBegin[i+1]-1;
		quickSort(RankedAdj, begin, end);
	}
}


void graph::rank_by_degree(){
	RankedBegin = new index_t[vertex_count+1];
	RankedBegin[0] = 0;

	// Calculate ranked begining position
	#pragma omp parallel for num_threads(56) schedule(dynamic,1024)
	for (vertex_t i = 0; i < vertex_count; i++){
		RankedBegin[i+1] = 0;
		//Calculte the degree of head vertex
		vertex_t h = head_list[beg_pos[i]];
		index_t dh = beg_pos[h+1]-beg_pos[h];
		for (index_t j = beg_pos[i]; j < beg_pos[i+1]; j++){
			//Calculte the degree of adj vertex
			vertex_t a = adj_list[j];
			index_t da = beg_pos[a+1] - beg_pos[a];
			if(dh<da || (dh==da && h<a)) RankedBegin[i+1]++;
		}
	}

	// If we put this step in the above loop, we cannot make sure sycn.
	for (int i = 0; i < vertex_count; i++){
		RankedBegin[i+1] += RankedBegin[i];	
	}

	// Calculate the RankedHead and RankedAdj
	RankedEdgeCount = RankedBegin[vertex_count];
	cout <<"RankedEdgeCount: "<< RankedEdgeCount << endl;
	RankedHead = new vertex_t[RankedEdgeCount];
	RankedAdj = new vertex_t[RankedEdgeCount];

	#pragma omp parallel for num_threads(56) schedule(dynamic,1024)
	for (vertex_t i = 0; i < vertex_count; i++){
		vertex_t h = head_list[beg_pos[i]];
		int dh = beg_pos[h+1]-beg_pos[h];
		int k = RankedBegin[i];

		for (index_t j = beg_pos[i]; j < beg_pos[i+1]; j++){
			vertex_t a = adj_list[j];
			index_t da = beg_pos[a+1] - beg_pos[a];
			if(dh < da || (dh == da && h<a)){
				RankedHead[k] = head_list[j];
				RankedAdj[k] = adj_list[j];
				k++;
			}
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
}

