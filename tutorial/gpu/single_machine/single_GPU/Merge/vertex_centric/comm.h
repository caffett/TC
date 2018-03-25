#ifndef	COMM_HEADER
#define	COMM_HEADER
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define THDS_NUM     256 
#define BLKS_NUM     256 
#define WRAP_SIZE    32

typedef 	long int 	index_t;
typedef		int		vertex_t;

inline off_t fsize(const char *filename) {
	struct stat st;
	if (stat(filename, &st) == 0){
		return st.st_size;
	}
	return -1;
}


static void HandleError(cudaError_t err,
                 const char *file,
                 int line ) 
{
    if (err != cudaSuccess) {
        printf( "%s in %s at line %d\n", \
              cudaGetErrorString( err ), file, line);
        exit(EXIT_FAILURE);
    }
}
#define H_ERR(err) (HandleError( err, __FILE__, __LINE__ ))

#endif
