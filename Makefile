SRC_DIR = src
BUILD_DIR = build/debug
CC = g++
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_NAME = play
INCLUDE_PATHS = -I/opt/homebrew/include/SDL2
LIB_PATHS = -L/opt/homebrew/lib
# /opt/homebrew/Cellar/sdl2/2.32.10/lib -L/opt/homebrew/Cellar/sdl2_image/2.8.8/lib
COMPILER_FLAGS = -std=c++17 -Wall -O0 -g
LINKER_FLAGS = -lsdl2 -lsdl2_image -lSDL2_ttf -lSDL2_mixer

all:
	$(CC) $(COMPILER_FLAGS) $(INCLUDE_PATHS) $(LIB_PATHS) $(SRC_FILES) -o $(BUILD_DIR)/$(OBJ_NAME) $(LINKER_FLAGS)
