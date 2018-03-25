EXE=tc

CUFLAGS= -O3 --compiler-options -Wall -Xptxas -v -lpthread

CUCC= "$(shell which nvcc)"

CUFLAGS+=  -Xcompiler -fopenmp


OBJS=   main.o 
DEPS= 	graph.h\
		graph.cpp\
		main.cpp\
		comm.h

%.o:%.cu $(DEPS)
	${CUCC} -c  ${CUFLAGS} $< -o $@

${EXE}:${OBJS}
	${CUCC} ${OBJS} $(CUFLAGS) -o ${EXE}

clean:
	rm -rf *.o ${EXE}
test:
	./tc ~/data/another_toy
