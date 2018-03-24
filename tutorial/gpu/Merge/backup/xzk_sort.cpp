void quickSort(vertex_t*, index_t, index_t)；


void graph::sort(){
	// sort the adj_list
	for (vertex_t i; i < vertex_count; i++){
		int begin = begin_pos[i];
		int end = begin_pos[i+1]-1;
		quickSort(adj_list, begin, end);
	}

	//Claculate the upper edge
	upperBegin = new vertex_t[vertex_count];
	upperBegin[0] = 0;
	upperEdgeCount = 0;

	for (int i = 0; i < vertex_count; i++){
		upperBegin[i+1] = upperBegin[i];
		for (int j = begin_pos[i]; j < begin_pos[i+1]; j++){
			if(adj_list[j]==adj_list[j+1]&&head_list[j]==head_list[j+1]) continue;

			if(head_list[j]<adj_list[j]){
				upperEdgeCount++;
				upperBegin[i+1]++;
			}
		}
	}

	upperAdj	= new vertex_t[upperEdgeCount];
	upperHead	= new vertex_t[upperEdgeCount];
	int k = 0;

	for (int i = 0; i < vertex_count; i++){
		upperBegin[i+1] = upperBegin[i];
		for (int j = begin_pos[i]; j < begin_pos[i+1]; j++){
			// Duplicate Edge?
			if(adj_list[j]==adj_list[j+1]&&head_list[j]==head_list[j+1]) continue;

			if(head_list[j]<adj_list[j]){
				upperHead[k] =head_list[j];
				upperAdj[k] =adj_list[j];
				k++;
			}
		}
	}
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