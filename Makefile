### Makefile for EE 5301 MP2 ###

CXXFLAGS	= -std=c++11 -Wall -Wextra -O2
THREADFLAGS	= -pthread
CXX		= g++ $(CXXFLAGS)


placement: placement.o libckt.o util.o librow.o 
	$(CXX) -o $@ $^

placement.o: placement.cpp libckt.hpp librow.hpp util.hpp
	$(CXX) -c $<

libckt.o: libckt.cpp libckt.hpp
	$(CXX) -c $<

util.o: util.cpp libckt.hpp librow.hpp util.hpp
	$(CXX) $(THREADFLAGS) -c $<

librow.o: librow.cpp librow.hpp libckt.hpp util.hpp
	$(CXX) -c $<

.PHONY: clean tarball

clean:
	rm -f *.o placement *~ *.txt *#

tarball: clean
	tar --exclude='.[^/]*' -zcvf ../MP2_chen5202.tgz ./
