CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++98
.PHONY: all test clean
all: physics_tests
physics_tests: ../Physics2D.cpp physics_tests.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp physics_tests.cpp -o physics_tests
test: physics_tests
	./physics_tests
clean:
	rm -f physics_tests
