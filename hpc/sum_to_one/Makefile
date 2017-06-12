CC = mpicc

NPROCS = 5

RUN = mpirun -np $(NPROCS)

compile: sum_to_one.c my_sum_to_one.c
	$(CC) -o sum_to_one sum_to_one.c my_sum_to_one.c

run:	compile
	$(RUN) sum_to_one

clean:
	rm *.o sum_to_one

