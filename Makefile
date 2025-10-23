# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -I../include
SDL_FLAGS = $(shell pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer)

# Target
TARGET = play

<<<<<<< Updated upstream
# Nguồn (các file trong src/)
SRCS = main.cpp game.cpp renderer.cpp
=======
# Source files
SRCS = main.cpp
LIB_SRCS = $(addprefix ../lib/, game.cpp renderer.cpp sudoku.cpp)
>>>>>>> Stashed changes

# Directory containing source
SRC_DIR = src

.PHONY: all clean run

# Build
all:
	@cd $(SRC_DIR) && \
	$(CXX) $(SRCS) $(LIB_SRCS) $(CXXFLAGS) $(SDL_FLAGS) -o $(TARGET)

run:
	@cd $(SRC_DIR) && ./$(TARGET)

clean:
	@cd $(SRC_DIR) && rm -f $(TARGET)
	@echo "Cleaned."
