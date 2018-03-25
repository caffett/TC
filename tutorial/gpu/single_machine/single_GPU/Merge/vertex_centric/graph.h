//graph.h
//Graph format: Json based format: [src_id, src_weigh,[[connected_ver_0, edge_weight],[connected_ver_1, edge_weight],[connected_ver_2, edge_weight]]]
//Storage format: 
//struct{
//		int: src_ver
//		Arr: [ver_0|ver_1|ver_2|...]
//		Int: num_conn_ver
//}
#ifndef	GRAPH_H
#define	GRAPH_H
#include "comm.h"

class graph{
public:
	vertex_t 	vertex_count;
	index_t		edge_count;
	vertex_t	*adj_list;
	vertex_t	*head_list;
	index_t	*beg_pos;

	//after rank by degree
	vertex_t	*RankedAdj;
	vertex_t	*RankedHead;
	index_t		*RankedBegin;
	index_t		RankedEdgeCount;

	index_t		*count;	

	graph(std::string filename);
	~graph();

	void sort_ranked_adj();
	void rank_by_degree();
	void scan();
};

#endif
