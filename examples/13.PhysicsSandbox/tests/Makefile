CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++98
.PHONY: all test clean
all: physics_tests menu_tests hud_tests failure_tests
physics_tests: ../Physics2D.cpp physics_tests.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp physics_tests.cpp -o physics_tests
menu_tests: menu_tests.cpp ../MenuFlow.h
	$(CXX) $(CXXFLAGS) -I.. menu_tests.cpp -o menu_tests
hud_tests: hud_tests.cpp ../HudStats.h ../StackProgress.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp hud_tests.cpp -o hud_tests
failure_tests: failure_tests.cpp ../TowerFailureRule.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp failure_tests.cpp -o failure_tests
test: physics_tests menu_tests hud_tests failure_tests
	./physics_tests
	./menu_tests
	./hud_tests
	./failure_tests
clean:
	rm -f physics_tests menu_tests hud_tests failure_tests
