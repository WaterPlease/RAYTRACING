CC=g++
LIBFLAGS = -lm -lGL -lGLU -lglut -lgsl -lgslcblas -lm -lpthread
OPTIONS = -Wno-pragma-once-outside-header
OBJS = sceneView.o

sceneView : sceneView.cpp *.hpp
	rm -f $(OBJS) ./sceneView
	$(CC) -o $@ $^ $(LIBFLAGS) -mavx2

clean :
	rm -f $(OBJS) ./scene/sceneView
