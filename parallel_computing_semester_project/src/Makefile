
# prism
#MPIDIR = /usr/lib/mpich/build/LINUX/ch_p4
# eyes
#MPIDIR = /usr/local/mpich-1.2.4/ch_p4
# leenooks
MPIDIR = /opt/mpi

MPILIBPATH = $(MPIDIR)/lib
MPIINCPATH = -I$(MPIDIR)/include \
             -I/usr/lib/mpich/include

MPI_INC = $(MPIINCPATH)
MPI_LIB = -L$(MPILIBPATH) -lmpich -lpmpich

GL_LIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lX11 -lm -lXmu -lXt -lXi -lXext

LIBS    = $(MPI_LIB) -lpthread

CFLAGS=-I/usr/X11R6/include $(MPI_INC) -D_DEBUG
LDFLAGS=-L/usr/lib $(LIBS) 

SEQ_OBJS= camera.o volume.o trace.o timer.o volren_seq.o

PAR_OBJS= camera.o timer.o mpilib.o myrandom.o lrudiskcache.o \
			lrumemcache.o lruheap.o pvolume.o cachemanager.o renderslave.o \
			loadbalancer.o volren_par.o

.cpp.o:
	g++ $(CFLAGS) -c $<

default:
	make -j2 sequential
	make -j2 parallel
	make -j2 data_dist
	make -j2 interfe
	make raw2rawiv

interfe: interfe.o invertMatrix.o
	g++ -o interfe interfe.o invertMatrix.o $(LDFLAGS) $(GL_LIBS)

sequential: $(SEQ_OBJS)
	g++ -o volren_seq $(SEQ_OBJS) $(LDFLAGS)

parallel: $(PAR_OBJS)
	g++ -o volren_par $(PAR_OBJS) $(LDFLAGS)

data_dist: data_dist.o timer.o myrandom.o
	g++ -o data_dist data_dist.o timer.o myrandom.o $(MPI_LIB)

raw2rawiv: raw2rawiv.o
	g++ -o raw2rawiv raw2rawiv.o

image_test: imagetest.o myrandom.o mpilib.o
	g++ -o image_test imagetest.o myrandom.o mpilib.o $(MPI_LIB)

clean:
	rm -f *.o interfe volren_par volren_seq data_dist raw2rawiv

