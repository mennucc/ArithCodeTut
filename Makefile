CXXFLAGS = -g -Wall -Wshadow  -Werror -Wunused -Wno-write-strings -Wno-format
LDFLAGS = -lm

all :  arith_simple arith_simple_test arith_test  arith_testV     arith_file   arith_fileV     arith_file_2   arith_file_2V    arith_inv arith_invV


#######
arith_simple: arith_simple.cc arith_code.cc Makefile
	${CXX} ${CXXFLAGS}  ${LDFLAGS} -g -O  arith_simple.cc  -o arith_simple

.PHONY: arith_simple_test
arith_simple_test: arith_simple
	cat /etc/passwd | ./arith_simple -C | ./arith_simple -D | cmp - /etc/passwd

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

########

#optimized
arith_file_2 : arith_file_2.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  -O arith_file_2.cc -o arith_file_2

#profile (not built by default)
arith_file_2P : arith_file_2.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_file_2.cc -o arith_file_2P

#verbose
arith_file_2V : arith_file_2.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  -DAC_VERBOSE -DVERBOSE  arith_file_2.cc -o arith_file_2V


#########

#optimized
arith_inv : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  -O arith_inv.cc -o arith_inv

#profile (not built by default)
arith_invP : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_inv.cc -o arith_invP

#verbose
arith_invV : arith_inv.cc arith_code.cc Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g  -DAC_VERBOSE -DVERBOSE  arith_inv.cc -o arith_invV


#########

clean:
	rm arith_test  arith_testP  arith_testV    arith_file arith_fileP arith_fileV     arith_file_2   arith_file_2V   arith_inv  arith_invO
