# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Ofast -flto -mbmi2 -mavx2 -msse4.1 -msse3 -msse2 -msse

# Define platform-specific flags
ifeq ($(OS), Windows_NT)
    # Windows-specific flags
    DEFINES = -DUSE_AVX2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE -DWIN32
    RM = del /Q
    EXECUTABLE_EXTENSION = .exe
else
    # Linux-specific flags
    DEFINES = -DUSE_AVX2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE -DLINUX
    RM = rm -f
    EXECUTABLE_EXTENSION =
endif

# Source files, object files, and executable
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = Thrawn.2.0linux$(EXECUTABLE_EXTENSION)

# Default target
all: $(EXECUTABLE)

# Link object files to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) -c -o $@ $<

# Clean up generated files
clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
