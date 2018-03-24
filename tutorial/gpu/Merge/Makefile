EXE=tc

COMMFLAGS= -O3 --compiler-options -Wall -Xptxas -v
CUCC= "$(shell which nvcc)"

CUFLAGS= -arch=sm_35  ${COMMFLAGS}
CUFLAGS+= -ccbin=g++ -Xcompiler -fopenmp

OBJS=	graph.o\
		main.o
DEPS=	graph.h\
		comm.h\
		wtime.h\

%.o:%.cu $(DEPS)
	${CUCC} -c  ${CUFLAGS} $< -o $@

${EXE}:${OBJS}
	${CUCC} ${OBJS} $(CUFLAGS) -o ${EXE}

clean:
	rm -rf *.o ${EXE}

test:
	./tc /home/zikang/data/another_toy
