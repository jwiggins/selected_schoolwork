ATLASPATH = /u/rvdg/ATLAS
ATLAS = $(ATLASPATH)/libf77blas.a $(ATLASPATH)/libcblas.a $(ATLASPATH)/libatlas.a 

LIB = $(ATLAS)

CC     = gcc
LINKER = gcc
CFLAGS = -I $(ATLASPATH) -DStringSunStyle -funroll-all-loops -fomit-frame-pointer -O3 -finline-functions
LFLAGS = $(CFLAGS)

util = fill_matrix.o \
       matrix_compare.o \
       copy_matrix.o \
       dirty_cache.o \
       dclock_s.o

gemm = test_gemm.o \
       my_gemm.o

test_gemm: $(gemm) $(util) Makefile
	$(LINKER) -o test_gemm.x $(LFLAGS) $(gemm) $(util) $(LIB)

clean: 
	rm -f *.o *~ core *.x




