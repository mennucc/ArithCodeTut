CXXFLAGS = -g -Wall -Wshadow  -Werror
LDFLAGS = -lm

CPROGRAMS = arith_simple arith_simple_v arith_simple_markov  arith_simple_markov_v arith_file   arith_file_v     arith_file_2   arith_file_2_v   arith_file_3   arith_file_3_v   arith_inv

# the three files that comprise the Arithmetic Coding library
LIBCODE=arith_code.cc arith_code.hh arith_code_config.hh

all : ${CPROGRAMS}  arith_simple_test arith_simple_markov_test

V=-DAC_COLORED -DAC_VERBOSE -DVERBOSE


#######
arith_simple: arith_simple.cc ${LIBCODE} frequencies.cc  Makefile
	${CXX} ${CXXFLAGS}  ${LDFLAGS} -g -O  arith_simple.cc  -o arith_simple

.PHONY: arith_simple_test
arith_simple_test: arith_simple
	cat /etc/passwd | ./arith_simple -C | ./arith_simple -D | cmp - /etc/passwd

arith_simple_v: arith_simple.cc ${LIBCODE}  frequencies.cc Makefile
	${CXX} ${CXXFLAGS}  ${LDFLAGS}  ${V} -g -O  arith_simple.cc  -o arith_simple_v

########

arith_simple_markov: arith_simple_markov.cc  ${LIBCODE}  frequencies.cc Makefile
	${CXX} ${CXXFLAGS}  ${LDFLAGS} -g -O  arith_simple_markov.cc  -o arith_simple_markov

.PHONY: arith_simple_markov_test
arith_simple_markov_test: arith_simple_markov
	cat /etc/passwd | ./arith_simple_markov -C | ./arith_simple_markov -D | cmp - /etc/passwd

arith_simple_markov_v: arith_simple_markov.cc ${LIBCODE}  frequencies.cc Makefile
	${CXX} ${CXXFLAGS}  ${LDFLAGS}  ${V} -g -O  arith_simple_markov.cc  -o arith_simple_markov_v


########

#optimized
arith_file : arith_file.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O arith_file.cc -o arith_file

#profile (not built by default)
arith_file_p : arith_file.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_file.cc -o arith_file_p

#verbose
arith_file_v : arith_file.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O ${V}  arith_file.cc -o arith_file_v

########

#optimized
arith_file_2 : arith_file_2.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O arith_file_2.cc -o arith_file_2

#profile (not built by default)
arith_file_2_p : arith_file_2.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_file_2.cc -o arith_file_2_p

#verbose
arith_file_2_v : arith_file_2.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O ${V} arith_file_2.cc -o arith_file_2_v

########

#optimized
arith_file_3 : arith_file_3.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O arith_file_3.cc -o arith_file_3

#profile (not built by default)
arith_file_3_p : arith_file_3.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_file_3.cc -o arith_file_3_p

#verbose
arith_file_3_v : arith_file_3.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O ${V} arith_file_3.cc -o arith_file_3_v


#########

#optimized
arith_inv : arith_inv.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O arith_inv.cc -o arith_inv

#profile (not built by default)
arith_inv_p : arith_inv.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -p -fprofile-arcs -ftest-coverage  arith_inv.cc -o arith_inv_p

#verbose
arith_inv_v : arith_inv.cc ${LIBCODE} Makefile ;
	 ${CXX} ${CXXFLAGS} ${LDFLAGS} -g -O ${V} arith_inv.cc -o arith_inv_v


#########

clean:
	rm   -f ${CPROGRAMS}
