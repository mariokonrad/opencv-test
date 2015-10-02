.PHONY: all clean

CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra

all : cam

cam : cam.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ `pkg-config --libs opencv --cflags`

cam-c : cam-c.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ `pkg-config --libs opencv --cflags`

clean :
	rm -f *.o
	rm -f cam
	rm -f cam-c
