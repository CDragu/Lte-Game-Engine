CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -Werror -pedantic -std=c++98
TESTS = physics_tests menu_tests hud_tests failure_tests tuning_tests sound_tests \
	audio_backend_tests weather_tests vertex_colors_tests robustness_tests
.PHONY: all test clean
all: $(TESTS)
physics_tests: ../Physics2D.cpp physics_tests.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp physics_tests.cpp -o physics_tests
menu_tests: menu_tests.cpp ../MenuFlow.h
	$(CXX) $(CXXFLAGS) -I.. menu_tests.cpp -o menu_tests
hud_tests: hud_tests.cpp ../HudStats.h ../GameRules.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp hud_tests.cpp -o hud_tests
failure_tests: failure_tests.cpp ../GameRules.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp failure_tests.cpp -o failure_tests
tuning_tests: tuning_tests.cpp ../Tuning.cpp ../Tuning.h ../GameRules.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Tuning.cpp ../Physics2D.cpp tuning_tests.cpp -o tuning_tests
sound_tests: sound_tests.cpp ../SoundSynth.cpp ../SoundSynth.h ../GameRules.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../SoundSynth.cpp ../Physics2D.cpp sound_tests.cpp -o sound_tests
audio_backend_tests: audio_backend_tests.cpp ../GameSounds.cpp ../GameSounds.h ../SoundSynth.cpp ../SoundSynth.h stubs/pspaudiolib.h stubs/pspkernel.h
	$(CXX) $(CXXFLAGS) -Istubs -I.. ../GameSounds.cpp ../SoundSynth.cpp audio_backend_tests.cpp -o audio_backend_tests
weather_tests: weather_tests.cpp ../Weather.cpp ../Weather.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Weather.cpp ../Physics2D.cpp weather_tests.cpp -o weather_tests
vertex_colors_tests: vertex_colors_tests.cpp ../BlockColors.h
	$(CXX) $(CXXFLAGS) -I.. vertex_colors_tests.cpp -o vertex_colors_tests
robustness_tests: robustness_tests.cpp ../GameRules.h ../Physics2D.cpp ../Physics2D.h ../Weather.cpp ../Weather.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp ../Weather.cpp robustness_tests.cpp -o robustness_tests
test: $(TESTS)
	@set -e; for test in $(TESTS); do ./$$test; done
clean:
	rm -f $(TESTS) tuning_test_output.cfg
