# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -I../include
SDL_FLAGS = $(shell pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf)

# Target
TARGET = play

# Lib files
LIB_SRCS = $(addprefix ../lib/, game.cpp renderer.cpp sudoku.cpp)

# Source files
SRCS = main.cpp 

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
