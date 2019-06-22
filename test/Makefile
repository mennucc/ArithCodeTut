all :  arith_simple arith_simple_test    arith_file   arith_fileV     arith_file_2   arith_file_2V    arith_inv arith_invV

########

#optimized
arith_test : arith_test.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -g -O  arith_test.cc  -o arith_test

#profile (not built by default)
arith_testP : arith_test.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage   arith_test.cc -o arith_testP
#verbose
arith_testV : arith_test.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -DAC_VERBOSE -DVERBOSE arith_test.cc -o arith_testV

clean:
	rm arith_test  arith_testP  arith_testV 
