CXX      = g++
CXXFLAGS = -Wall -Wextra -g3 -static-libgcc -static-libstdc++

BUILD_DIR = build
OBJ_DIR   = build\obj

SRCS = src/main.cpp
OBJS = $(BUILD_DIR)/main.o
EXE  = $(BUILD_DIR)/main.exe

all: dirs $(EXE)

dirs:
	cmd /c if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	cmd /c if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	cmd /c build\main.exe

clean:
	cmd /c if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)

.PHONY: all dirs run clean
