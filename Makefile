
# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -I../include
SDL_FLAGS = $(shell pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer)

# Target
TARGET = sudoku

# Source files
SRCS = main.cpp game.cpp renderer.cpp sudoku.cpp

# Thư mục chứa source
SRC_DIR = src

.PHONY: all clean run

# Build (có thể chạy lệnh make từ trong src hoặc thư mục cha đều được)
all:
	@cd $(SRC_DIR) && \
	$(CXX) $(SRCS) $(CXXFLAGS) $(SDL_FLAGS) -o $(TARGET)

run:
	@cd $(SRC_DIR) && ./$(TARGET)

clean:
	@cd $(SRC_DIR) && rm -f $(TARGET)
	@echo "Cleaned."
