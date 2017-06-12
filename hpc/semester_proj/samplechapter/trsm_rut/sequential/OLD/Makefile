include /v/hank/v102/rvdg/FLAME/Make.include

ltrsm =  test_ltrsm_4_16.o \
	 fill_matrix.o \
	 Trsm_right_lower_trans.o \
	 Trsm_right_lower_trans_lev2.o \
	 Trsm_right_lower_trans_recurse.o \
	 Trsm_right_lower_trans2.o \
     Trsm_right_lower_trans2_lev2.o \
     Trsm_right_lower_trans2_recurse.o

test_ltrsm: $(ltrsm) $(FLAMELIB) Makefile
	$(LINKER) -o test_ltrsm.x $(LFLAGS) $(ltrsm) $(LIB)

clean: 
	rm -f *.o *~ core *.x
