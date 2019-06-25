all :  arith_test_c   arith_test_c_v   arith_test_p   arith_test_p_v

C=-DENCODE_BY_CALLBACK -DDECODE_BY_CALLBACK

V=-DAC_COLORED -DAC_VERBOSE -DVERBOSE

######## using callbacks

#optimized
arith_test_c : arith_test.cc ../arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} ${C} -g -O  arith_test.cc  -o arith_test_c

#profile (not built by default)
arith_test_c_p : arith_test.cc ../arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} ${C} -p -fprofile-arcs -ftest-coverage   arith_test.cc -o arith_test_c_p
#verbose
arith_test_c_v : arith_test.cc ../arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} ${C} -g ${V} arith_test.cc -o arith_test_c_v

####### without callbacks, using polling
#optimized
arith_test_p : arith_test.cc ../arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -g -O  arith_test.cc  -o arith_test_p

#profile (not built by default)
arith_test_p_p : arith_test.cc ../arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage   arith_test.cc -o arith_test_p_p
#verbose
arith_test_p_v : arith_test.cc ../arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  ${V} arith_test.cc -o arith_test_p_v


clean:
	rm arith_test_c  arith_test_c_p  arith_test_c_v   arith_test_p  arith_test_p_p  arith_test_p_v
