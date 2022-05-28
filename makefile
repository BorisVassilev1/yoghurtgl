SHELL = /bin/sh

SRC_DIR := ./src
OBJ_DIR := ./obj
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
DEP_FILES := $(wildcard ./include/*.h)
#OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))


result_file := main
libs :=
includes := -I"./include"
std := -std=c++2a
rmcmd := rm
CC := 

ifeq ($(OS),Windows_NT)
    CC := g++ # why does this not compile if I try to do it with clang
	result_file := main.exe
	includes += -I"D:/cpp_libraries/glfw-3.3.5-source/include" -I"D:/cpp_libraries/glew-2.1.0-source/include" -I"D:/cpp_libraries/glm"
	libs += -L"D:/cpp_libraries/glfw-3.3.5-compiled/src" -L"D:/cpp_libraries/glew-2.1.0-compiled/lib"
	libs += -lglfw3 -lglew32 -lopengl32
	rmcmd := if exist $(result_file) del $(result_file)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		CC := g++
        result_file := main
		libs += -lglfw -lGL -lX11 -lGLEW
		rmcmd := rm -f $(result_file)
	else 
		$(error Unknown or unsupported OS)
	endif
endif

all: $(result_file)

$(result_file): main.cpp makefile $(SRC_FILES) $(DEP_FILES)
	$(CC) main.cpp $(SRC_FILES) -g -o $(result_file) $(libs) $(includes) $(std) -Wall

.PHONY : clean
clean :
	-$(rmcmd)

-include $(OBJ_FILES:.o=.d)