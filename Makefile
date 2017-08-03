CFLAGS=-I/usr/local/include -L/usr/local/lib -std=c++11
LFLAGS=-lopencv_core -lopencv_highgui -lopencv_imgproc

all:
	$(CXX) $(CFLAGS) -o prog main.cpp $(LFLAGS)

clean:
	rm prog
	rm -rf output/*
