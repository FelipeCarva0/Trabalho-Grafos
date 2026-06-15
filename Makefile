CXX      = C:/msys64/ucrt64/bin/g++.exe
CXXFLAGS = -Wall -Wextra -g3
INCLUDES = -IC:/msys64/ucrt64/include/opencv4
LIBS     = -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc

BUILD_DIR = build
OUT_DIR   = output
SRC_DIR   = src

all: dirs solucaoA solucaoB solucaoC

dirs:
	cmd /c if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	cmd /c if not exist $(OUT_DIR) mkdir $(OUT_DIR)

solucaoA: dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC_DIR)/main.cpp $(SRC_DIR)/solucaoA.cpp -o $(OUT_DIR)/solucaoA.exe $(LIBS)

solucaoB: dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC_DIR)/main.cpp $(SRC_DIR)/solucaoB.cpp -o $(OUT_DIR)/solucaoB.exe $(LIBS)

solucaoC: dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC_DIR)/main.cpp $(SRC_DIR)/solucaoC.cpp -o $(OUT_DIR)/solucaoC.exe $(LIBS)

runA: solucaoA
	cmd /c output\solucaoA.exe

runB: solucaoB
	cmd /c output\solucaoB.exe

runC: solucaoC
	cmd /c output\solucaoC.exe

clean:
	cmd /c if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	cmd /c if exist $(OUT_DIR) rmdir /s /q $(OUT_DIR)

.PHONY: all dirs solucaoA solucaoB solucaoC runA runB runC clean