FLAMEPATH = /u/rvdg/FLAME

include $(FLAMEPATH)/Make.include

sytrrk = test_sytrrk.o \
         fill_matrix.o \
	 dclock_s.o \
	 REF_Sytrrk.o \
	 MY_Sytrrk_uln_unblk.o \
	 MY_Sytrrk_uln_blk.o

test_sytrrk: $(sytrrk) $(FLAMELIB) Makefile
	$(LINKER) -o test_sytrrk.x $(LFLAGS) $(sytrrk) $(LIB)

clean: 
	rm -f *.o *~ core *.x
