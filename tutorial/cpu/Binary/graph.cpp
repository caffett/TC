//Graph format: 
//Simplified json format: 
//src degree dest0 dest1 ...

#include "graph.h"
#include "comm.h"
#include <fstream>
#include <omp.h>

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

	vert_count = fsize(begin_file)/sizeof(index_t) - 1;
	edge_count = fsize(head_file)/sizeof(vertex_t);
	cout<<"vert:"<< vert_count << "  edge: " << edge_count<<endl;


	FILE *pFile= fopen(adj_file, "rb");
	adj_list = (vertex_t *)malloc(fsize(adj_file));
	fread(adj_list, sizeof(vertex_t), edge_count, pFile);
	fclose(pFile);

	FILE *pFile1= fopen(head_file,"rb");
	head_list = (vertex_t *)malloc(fsize(head_file));
	fread(head_list,sizeof(vertex_t),edge_count,pFile1);
	fclose(pFile1);

	FILE *pFile3 = fopen(begin_file,"rb");
	beg_pos = (index_t *)malloc(fsize(begin_file));
	fread(beg_pos,sizeof(index_t),vert_count+1,pFile3);
	fclose(pFile3);
}


void graph::rank_by_degree(){
	upperBegin	= new index_t[vert_count+1];
	upperBegin[0]=0;
	index_t k=0;
	for(vertex_t i=0; i<vert_count; i++){
		upperBegin[i+1]=upperBegin[i];//upperDegree[i]=0;
		index_t j=beg_pos[i];
			vertex_t h=head_list[j];
			index_t dh=beg_pos[h+1]-beg_pos[h];
		while(j<beg_pos[i+1]){
			vertex_t a=adj_list[j];
			index_t da=beg_pos[a+1]-beg_pos[a];
			if(dh<da || (dh==da && h<a)){
				k++;
				upperBegin[i+1]++;//upperDegree[i]++;
			}
			j++;
		}
	}
	
	upperEdgeCount = k;
	upperAdj	= new vertex_t[upperEdgeCount];
	upperHead	= new vertex_t[upperEdgeCount];
	k=0;
	for(vertex_t i=0; i<vert_count; i++){
		index_t j=beg_pos[i];
			vertex_t h=head_list[j];
			index_t dh=beg_pos[h+1]-beg_pos[h];
		while(j<beg_pos[i+1]){
			vertex_t a=adj_list[j];
			index_t da=beg_pos[a+1]-beg_pos[a];
			if(dh<da || (dh==da && h<a)){
				upperAdj[k] =adj_list[j];
				upperHead[k] =head_list[j];
				k++;
			}
			j++;
		}
	}
	
	cout<<"upper Edge Count= "<<upperEdgeCount<<"\n";
}


void graph::bsvalidation(){
	index_t mycount=0;
	#pragma omp parallel for num_threads(56) reduction(+:mycount) schedule(dynamic,512)
	for(index_t i=0; i<upperEdgeCount; i++){
		vertex_t A=upperHead[i];
		vertex_t B=upperAdj[i];
		index_t m=upperBegin[A+1]-upperBegin[A];
		index_t n=upperBegin[B+1]-upperBegin[B];
		
		// Find the smaller one as the looking-up table
		if(m<n){
			A=A^B;
			B=A^B;
			A=A^B;
			m=m^n;
			n=m^n;
			m=m^n;
		}

		// Binary-seracher based method.
		vertex_t *a = &upperAdj[upperBegin[A]];
		vertex_t *b = &upperAdj[upperBegin[B]];

		index_t bot;
		index_t top;
		index_t r;

		for(index_t j=0; j<n; j++){
			vertex_t X = b[j];
			vertex_t Y;
			bot = 0;
			top = m-1;
			
			while(top>=bot){
				r = (top+bot)/2;
				Y = a[r];
				if(X==Y){
					mycount++;
					break;
				}
				else if(X<Y){
					top = r-1;
				}
				else if(X>Y){
					bot = r+1;
				}
			}
		}

	}
	cout<<"bs validation version tc = "<<mycount<<endl;
}

graph::~graph(){
	delete[] adj_list;
	delete[] head_list;
	delete[] beg_pos;
	delete[] upperAdj;
	delete[] upperHead;
	delete[] upperBegin;
}
