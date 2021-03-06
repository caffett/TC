#ifndef	COMM_HEADER
#define	COMM_HEADER
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//--------------------------
//typedef 	unsigned long int 	index_t;
//typedef		unsigned int		vertex_t;

typedef 	long int 	index_t;
typedef		int		vertex_t;
//--------------------------------

//#define PartitionSize	268435456 //256M edges, 1G for csr
//#define PartitionSize	67108864 //64M edges, 256M for csr
#define PartitionSize	134217728 //128M edges, 512M for csr
//#define PartitionSize	33554432 //32M edges, 128M for csr
//#define PartitionSize	536870912 //512M edges, 2G for csr
//#define PartitionSize	3 //test
//#define BufferSize	16777216 //16M edges, or 64MB*2 for edge buffer
//#define PART_NUM 	2
int PART_NUM;
int _2DPART;


inline off_t fsize(const char *filename) {
	struct stat st;
	if (stat(filename, &st) == 0){
		return st.st_size;
	}
	return -1;
}


static void HandleError( cudaError_t err,
                         const char *file,
                         int line ) {
    if (err != cudaSuccess) {
        printf( "%s in %s at line %d\n", \
        cudaGetErrorString( err ),
                file, line );
        exit( EXIT_FAILURE );
    }
}
#define H_ERR( err ) \
  (HandleError( err, __FILE__, __LINE__ ))

typedef struct EDGE{
	vertex_t A;
	vertex_t B;
} Edge;
typedef struct GPUData{
	int id;
	int partition_id;

	int currentBuffer;

	vertex_t * adj;
	index_t  * begin;
	index_t  * count;
	
	Edge **EdgeBuffer;

} GPU_data;



#endif
