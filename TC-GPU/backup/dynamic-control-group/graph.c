//Graph format: 
//Simplified json format: 
//src degree dest0 dest1 ...


//#include "graph.h"
#include "comm.h"
#include "wtime.h"
#include <fstream>
#include <omp.h>

#define FILE_NOT_EXIST	1
#define FILE_EXIST	0

#define CPU_id GPU_NUM
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

	vert_count = fsize(begin_file)/sizeof(index_t) - 1;
	edge_count = fsize(head_file)/sizeof(vertex_t);

	cout<<"vert:"<< vert_count<<"  edge: "<<edge_count<<endl;


	FILE *pFile= fopen(adj_file,"rb");
	adj_list = (vertex_t *)malloc(fsize(adj_file));
	fread(adj_list,sizeof(vertex_t),edge_count,pFile);
	fclose(pFile);

	FILE *pFile1= fopen(head_file,"rb");
	head_list = (vertex_t *)malloc(fsize(head_file));
	fread(head_list,sizeof(vertex_t),edge_count,pFile1);
	fclose(pFile1);

//	FILE *pFile2= fopen(degree_file,"rb");
//	adj_card = (index_t *)malloc(fsize(degree_file));
//	fread(adj_card,sizeof(index_t),vert_count,pFile2);
//	fclose(pFile2);

	FILE *pFile3 = fopen(begin_file,"rb");
	beg_pos = (index_t *)malloc(fsize(begin_file));
	fread(beg_pos,sizeof(index_t),vert_count+1,pFile3);
	fclose(pFile3);

	count = new index_t[256];
//	valid = (int *)malloc(vert_count*sizeof(int));
	gdata = new GPU_data[GPU_NUM];
	for(int i=0; i<GPU_NUM; i++){
		gdata[i].id = i;
		gdata[i].EdgeBuffer = new Edge* [2];
	}

	ds_help		= new index_t [DEV_NUM];
	ds_complete	= new index_t [DEV_NUM];
	ds_last		= new index_t [DEV_NUM];
}

/*
void graph::validation(){
	int mycount=0;
	for(int i=0; i<upperEdgeCount; i++){
		int U=upperHead[i];
		int V=upperAdj[i];
		int m=upperDegree[U];
		int n=upperDegree[V];

		int *u = &upperAdj[upperBegin[U]];
		int *v = &upperAdj[upperBegin[V]];

		int u1=0;
		int v1=0;
		while(u1<m && v1<n){
			int x=u[u1];
			int y=v[v1];
			if(x<y){
				u1++;
			}
			else if(x>y){
				v1++;
			}
			else if(x==y){
				u1++;
				v1++;
				mycount++;
			}
		}
	}
	printf("validation version tc = %d\n",mycount);
}
*/

void graph::cpuCompute(int Part_id, index_t Chunk_id){
	int P = Part_id;

	index_t mycount=0;
	vertex_t * adj = partAdj[P];
	index_t * begin = partBegin[P]; 
	index_t currentBufferSize = BufferSize;
	if(Chunk_id==upperEdgeCount/BufferSize){
		currentBufferSize = upperEdgeCount % BufferSize;
	}
	Edge* workload = &OrientedEdge[Chunk_id*BufferSize];
//#pragma omp parallel for num_threads(56) reduction(+:mycount) schedule(dynamic,1024)
#pragma omp parallel for reduction(+:mycount) schedule(dynamic,1024)
	for(index_t i=0; i<currentBufferSize; i++){
		vertex_t A=workload[i].A;
		vertex_t B=workload[i].B;
		index_t m=begin[A+1]-begin[A];
		index_t n=begin[B+1]-begin[B];

//cout<<"edge: "<<i<<" "<<U<<"-"<<V<<" ";
//cout<<"degree: "<<m<<" "<<n<<endl;

		vertex_t *a = &adj[begin[A]];
		vertex_t *b = &adj[begin[B]];

		vertex_t u1=0;
		vertex_t v1=0;
		while(u1<m && v1<n){
			vertex_t x=a[u1];
			vertex_t y=b[v1];
			if(x<y){
				u1++;
			}
			else if(x>y){
				v1++;
			}
			else if(x==y){
				u1++;
				v1++;
				mycount++;
			}
		}
	}
//	count[GPU_NUM] += mycount;
	ds_count[P * ChunkNum + Chunk_id] = mycount;
//	cout<<"merge version tc = "<<mycount<<endl;
}

void graph::cpuProc(){
double t0 = wtime();
       		
//	count[GPU_NUM] = 0;

//	bool help = 0;
	for(int P=0; P<PART_NUM; P++){
//		if(help){
//			break;
//		}
//	step 1: work from an initiate chunk id
		for(index_t i = CPU_id; i<ChunkNum; i+= DEV_NUM){
			if(ds_status[P*ChunkNum + i]!=0) {
//				help = 1;//set for outer-loop
//				break;
				return;
			}
			//finish with someone's help
			//
			ds_status[P*ChunkNum + i] = 1;
			ds_complete[CPU_id]++;
			cpuCompute(P,i);
//cout<<"CPU "<<CPU_id<<" chunk "<<i<<endl;
		}
	}
//cout<<"CPU complete = "<<ds_complete[CPU_id];
//step 2: work stealing
//---------------------
double t1 = wtime();
cout<<"CPU  time = "<<t1-t0<<endl;
}

void graph::reduceResult(){

	count[0]=0;
	for (int i=0; i<PART_NUM*ChunkNum; i++){
		count[0] += ds_count[i];
	}
}

