PLAPACK_ROOT = /archive/utexas/cs/rvdg/PLAPACKR31

include $(PLAPACK_ROOT)/Make.include

UTIL   = create_problem.o 

TEST_TRSM = test_trsm.o Trsm_right_upper_trans_blk.c

test_trmm: $(TEST_TRSM) $(UTIL)
	rm -f TEST_TRSM
	$(LINKER) -o $(WORK)/test_trmm.x $(LFLAGS) $(TEST_TRSM) $(UTIL) $(OBJS) \
        $(PLAPACKLIB) $(MMPILIB) $(lapackLIB) $(LIB)

clean :
	rm -f TEST_* *.o MMPI* core *~ file* *.x output_test_gemm *log

