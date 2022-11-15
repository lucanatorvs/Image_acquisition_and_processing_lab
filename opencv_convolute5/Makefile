#beun makefile by Roderik 

# g++ main.cpp -o main -L/usr/include/opencv2/ -lopencv_objdetect -lopencv_features2d -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_imgcodecs

MAIN = main
CFLAGS = -g -Wall -fno-omit-frame-pointer
LPATH = -L/usr/include/opencv2/
LIBS = -lopencv_objdetect -lopencv_features2d -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_imgcodecs

all: $(MAIN)

$(MAIN) : $(MAIN).cpp
	g++ $(CFLAGS) $(LIBS) -o $@ $<

clean:
	rm $(MAIN)
