CPP=g++
LDP=g++
LD_FLAGS = -lstdc++ -lopencv_core -lopencv_highgui -lopencv_imgproc -g
FLAGS = -I/usr/include/opencv -g
PROGC = testVideo.x
OBJSC = testVideo.o testVideo.o

RM = /bin/rm

#all rule
all: $(PROGC)

$(PROGC): $(OBJSC)
	$(LDP) $^ $(LD_FLAGS) -o $@
%.o: %.cpp
	$(CPP) $(FLAGS) -c $^ -o $@

#clean rule
clean:
	$(RM) -rf *.o $(PROGC)
