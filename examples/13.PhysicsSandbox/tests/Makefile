CXX ?= c++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++98
.PHONY: all test clean
all: physics_tests menu_tests hud_tests failure_tests tuning_tests sound_tests audio_backend_tests
physics_tests: ../Physics2D.cpp physics_tests.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp physics_tests.cpp -o physics_tests
menu_tests: menu_tests.cpp ../MenuFlow.h
	$(CXX) $(CXXFLAGS) -I.. menu_tests.cpp -o menu_tests
hud_tests: hud_tests.cpp ../HudStats.h ../StackProgress.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp hud_tests.cpp -o hud_tests
failure_tests: failure_tests.cpp ../TowerFailureRule.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Physics2D.cpp failure_tests.cpp -o failure_tests
tuning_tests: tuning_tests.cpp ../Tuning.cpp ../Tuning.h ../TuningPhysics.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../Tuning.cpp ../Physics2D.cpp tuning_tests.cpp -o tuning_tests
sound_tests: sound_tests.cpp ../SoundSynth.cpp ../SoundSynth.h ../SoundCues.h ../Physics2D.cpp ../Physics2D.h
	$(CXX) $(CXXFLAGS) -I.. ../SoundSynth.cpp ../Physics2D.cpp sound_tests.cpp -o sound_tests
audio_backend_tests: audio_backend_tests.cpp ../GameSounds.cpp ../GameSounds.h ../SoundSynth.cpp ../SoundSynth.h stubs/pspaudiolib.h stubs/pspkernel.h
	$(CXX) $(CXXFLAGS) -Istubs -I.. ../GameSounds.cpp ../SoundSynth.cpp audio_backend_tests.cpp -o audio_backend_tests
test: physics_tests menu_tests hud_tests failure_tests tuning_tests sound_tests audio_backend_tests
	./physics_tests
	./menu_tests
	./hud_tests
	./failure_tests
	./tuning_tests
	./sound_tests
	./audio_backend_tests
clean:
	rm -f physics_tests menu_tests hud_tests failure_tests tuning_tests sound_tests audio_backend_tests tuning_test_output.cfg
