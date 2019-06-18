CXXFLAGS = -g -Wall   -Werror -Wunused -Wno-write-strings -Wno-format
LDFLAGS = -lm

all :  arith_test  arith_testO   arith_testV
##arith_inv arith_invO

arith_test : arith_test.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage   arith_test.cc -o arith_test 

arith_testV : arith_test.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage -DAC_VERBOSE -DVERBOSE arith_test.cc -o arith_testV


arith_testO : arith_test.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -O2  arith_test.cc  -o arith_testO



arith_inv : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  -DAC_VERBOSE arith_inv.cc -o arith_inv

arith_invO : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -O2  arith_inv.cc -o arith_invO

clean:
	rm arith_test  arith_testO  arith_testV  arith_inv  arith_invO
