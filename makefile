# Name of the executable
TARGET = linker

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -O2

# Source file
SRCS = linker.cpp

# Build target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
