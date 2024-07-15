# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Ofast -flto

# Establish the operating system name
KERNEL := $(shell uname -s)
ifeq ($(KERNEL),Linux)
    OS := $(shell uname -o)
else ifeq ($(KERNEL),Darwin)
    OS := macOS
endif

# Executable name
ifeq ($(OS),Windows_NT)
    EXE = Thrawn.exe
else
    EXE = Thrawn
endif

# Source and object files
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Default optimization flags
OPTIMIZE_FLAGS = -O3 -funroll-loops

# Define architecture-specific flags
ARCH_FLAGS =

ifeq ($(KERNEL),Linux)
    ifeq ($(shell uname -m), x86_64)
        ARCH_FLAGS += -mavx512vnni -mavx256vnni -mavx512bw -mavxvnni -mavx2 -msse4.1 -msse3 -msse2 -mmmx -msse
    else ifeq ($(shell uname -m), aarch64)
        ARCH_FLAGS += -DARM64 -DUSE_NEON -DUSE_NEON_DOTPROD -flax-vector-conversions
    endif
else ifeq ($(KERNEL),Darwin)
    ARCH_FLAGS += -mmacosx-version-min=10.15
    ifeq ($(shell uname -m), arm64)
        ARCH_FLAGS += -DARM64 -DUSE_NEON -DUSE_NEON_DOTPROD -flax-vector-conversions
    else
        ARCH_FLAGS += -mavx512vnni -mavx256vnni -mavx512bw -mavxvnni -mavx2 -msse4.1 -msse3 -msse2 -mmmx -msse
    endif
else ifeq ($(OS),Windows_NT)
    ARCH_FLAGS += -DWIN32
    ifeq ($(PROCESSOR_ARCHITECTURE), AMD64)
        ARCH_FLAGS += -mavx512vnni -mavx256vnni -mavx512bw -mavxvnni -mavx2 -msse4.1 -msse3 -msse2 -mmmx -msse
    else ifeq ($(PROCESSOR_ARCHITECTURE), ARM64)
        ARCH_FLAGS += -DARM64 -DUSE_NEON -DUSE_NEON_DOTPROD -flax-vector-conversions
    endif
endif

# Default target
all: $(EXE)

# Link object files to create the executable
$(EXE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(ARCH_FLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(ARCH_FLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(OBJECTS) $(EXE)

# Detailed flags information
debug = no
sanitize = none
optimize = yes

ifeq ($(optimize),yes)
    CXXFLAGS += $(OPTIMIZE_FLAGS)
endif

ifeq ($(debug),yes)
    CXXFLAGS += -g
else
    CXXFLAGS += -DNDEBUG
endif

ifneq ($(sanitize),none)
    CXXFLAGS += -g3 $(addprefix -fsanitize=,$(sanitize))
    LDFLAGS += $(addprefix -fsanitize=,$(sanitize))
endif

# Default architecture if not set
ifeq ($(ARCH),)
   ARCH = native
endif

# Allow overwriting CXX from command line
ifdef COMPCXX
    CXX=$(COMPCXX)
endif

.PHONY: all clean
