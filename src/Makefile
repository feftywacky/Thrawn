# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Ofast -flto

# Define platform-specific flags
ifeq ($(OS), Windows_NT)
    # Windows-specific flags
    CXXFLAGS += -DWIN32
    ifeq ($(PROCESSOR_ARCHITECTURE), AMD64)
        CXXFLAGS += -DUSE_AVX2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE), ARM64)
        CXXFLAGS += -DARM64
    endif
    RM = del /Q
    EXECUTABLE_EXTENSION = .exe
else
    UNAME_S := $(shell uname -s)
    UNAME_M := $(shell uname -m)
    ifeq ($(UNAME_S), Darwin)
        # macOS-specific flags
        CXXFLAGS += -DMACOS
        ifeq ($(UNAME_M), arm64)
            CXXFLAGS += -DARM64
        else
            CXXFLAGS += -DUSE_AVX2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE -mbmi2 -mavx2 -msse4.1 -msse3 -msse2 -msse
        endif
        RM = rm -f
        EXECUTABLE_EXTENSION =
    else
        # Linux-specific flags
        CXXFLAGS += -DLINUX
        ifeq ($(UNAME_M), x86_64)
            CXXFLAGS += -DUSE_AVX2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE -mbmi2 -mavx2 -msse4.1 -msse3 -msse2 -msse
        else ifeq ($(UNAME_M), aarch64)
            CXXFLAGS += -DARM64
        endif
        RM = rm -f
        EXECUTABLE_EXTENSION =
    endif
endif

# Source files, object files, and executable
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = Thrawn_v2.0$(EXECUTABLE_EXTENSION)

# Default target
all: $(EXECUTABLE)

# Link object files to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
