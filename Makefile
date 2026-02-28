
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
SRC = src/Main.cpp src/Board.cpp src/Game.cpp src/player/GreedyAI.cpp src/player/MinimaxAI.cpp src/player/BeamAI.cpp
OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))
TARGET = build/chinese_checker

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp | build
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -f $(OBJ) $(TARGET)
