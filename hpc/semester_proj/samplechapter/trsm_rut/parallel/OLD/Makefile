include /archive/utexas/cs/rvdg/PLAPACKR31/Make.include

ptrsm =  test_trsm.o \
	 Trsm_right_lower_trans.o

test_trsm: $(ptrsm) $(FLAMELIB) Makefile
	$(LINKER) -o test_trsm.x $(LFLAGS) $(ptrsm) $(PLAPACKLIB) $(LIB)

clean: 
	rm -f *.o *~ core *.x

