include /v/netap/v0/kaycea/cs378/FLAME/Make.include

syrk =   test_syrk_unn.o \
         fill_matrix.o \
	 dirty_cache.o \
	 up-left/Syrk_unn_upleft_wrt_C_blk.c \
	 up-left/Syrk_unn_upleft_wrt_C_unb.c \
	 down-right/Syrk_unn_downright_wrt_C_blk.c \
	 down-right/Syrk_unn_downright_wrt_C_unb.c

test_syrk_un: $(syrk) $(FLAMELIB) Makefile
	$(LINKER) -o test_syrk_unn.x $(LFLAGS) $(syrk) $(LIB)

clean: 
	rm -f *.o *~ core *.x */*.o */*~ */*.x


