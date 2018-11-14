### Makefile for EE 5301 MP2 ###

CXXFLAGS	= -std=c++11 -Wall -Wextra -g
CXX		= g++ $(CXXFLAGS)


placement: placement.o libckt.o libckt.hpp
	$(CXX) -o $@ $^

placement.o: placement.cpp libckt.hpp
	$(CXX) -c $<

libckt.o: libckt.cpp libckt.hpp
	$(CXX) -c $<

.PHONY: clean

clean:
	rm -f *.o placement *~ *.txt *#
