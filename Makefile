.PHONY: all clean

CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -ggdb

all : cam motion

cam : cam.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ `pkg-config --libs opencv --cflags`

cam-c : cam-c.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ `pkg-config --libs opencv --cflags`

motion : motion.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ `pkg-config --libs opencv --cflags`

clean :
	rm -f *.o
	rm -f cam
	rm -f cam-c
	rm -f motion
