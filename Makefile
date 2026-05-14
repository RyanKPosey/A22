CXX = g++
CXXFLAGS = -std=c++17 -O2 -g
TARGET = sorts
OBJS = main.o sorts.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

run-all:
	./$(TARGET) -a 10000

.PHONY: all clean run-all
