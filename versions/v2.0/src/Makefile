CXX = g++
CXXFLAGS = -std=c++17 -Ofast -flto -mbmi2 -mavx2 -msse4.1 -msse3 -msse2 -msse
DEFINES = -DUSE_AVX2 -DUSE_SSE41 -DUSE_SSE3 -DUSE_SSE2 -DUSE_SSE

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = Thrawn

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)