CXXFLAGS = -g -Wall   -Werror -Wunused -Wno-write-strings -Wno-format
LDFLAGS = -lm

all :  arith_test  arith_testV    arith_file   arith_fileV
##arith_inv arith_invO

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

########

#optimized
arith_file : arith_file.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  -O arith_file.cc -o arith_file

#profile (not built by default)
arith_fileP : arith_file.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_file.cc -o arith_fileP

#verbose
arith_fileV : arith_file.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  -DAC_VERBOSE -DVERBOSE  arith_file.cc -o arith_fileV

#########

arith_inv : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  -DAC_VERBOSE arith_inv.cc -o arith_inv

arith_invO : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS}  ${LDFLAGS} -O2  arith_inv.cc -o arith_invO

clean:
	rm arith_test  arith_testP  arith_testV    arith_file arith_fileP arith_fileV  arith_inv  arith_invO
