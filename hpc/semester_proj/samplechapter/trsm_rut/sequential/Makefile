include ../../../../FLAME/Make.include
#include ~rvdg/FLAME_CLASS/FLAME/Make.include

trsm =   test_trsm_rut.o \
         fill_matrix.o \
	 dirty_cache.o \
	 dclock_s.o \
	 Trsm_right_upper_trans_unb.o \
	 Trsm_right_upper_trans_blk.o

test_trsm_rut: $(trsm) $(FLAMELIB) Makefile
	$(LINKER) -o test_trsm.x $(LFLAGS) $(trsm) $(LIB)

clean: 
	rm -f *.o *~ core *.x */*.o */*~ */*.x


