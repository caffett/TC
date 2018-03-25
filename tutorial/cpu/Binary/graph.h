//graph.h
//Graph format: Json based format: [src_id, src_weigh,[[connected_ver_0, edge_weight],[connected_ver_1, edge_weight],[connected_ver_2, edge_weight]]]
//Storage format: 
//struct{
//		int: src_ver
//		Arr: [ver_0|ver_1|ver_2|...]
//		Int: num_conn_ver
//	}
#ifndef	GRAPH_H
#define	GRAPH_H
#include "wtime.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>
#include "comm.h"


class graph{
public:
	vertex_t 	vert_count;
	vertex_t	*adj_list;
	vertex_t	*head_list;
	index_t	*beg_pos;

	//after ranking by degree
	vertex_t	*upperAdj;
	vertex_t	*upperHead;
	index_t	*upperBegin;
	index_t	*upperDegree;
	index_t	upperEdgeCount;
	
	index_t		edge_count;

	graph() {};
	graph(std::string filename);
	~graph();

	void validation();	//Merge-Based function to calcule the number of triangles.
	void bsvalidation();  //Binary Search function to calculate the nuber of triangles.
	void rank_by_degree();
};

#endif
