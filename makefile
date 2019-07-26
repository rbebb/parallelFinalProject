CC=mpicc
CPP=g++
LDC=mpicc
LDP=mpicc
LD_FLAGS = -fopenmp -lstdc++ -lopencv_core -lopencv_highgui -lopencv_imgproc
FLAGS= -fopenmp -I/usr/include/opencv
PROGC = videoProcessing.x
OBJSC = videoProcessing.o imageTools.o convolution.o

RM = /bin/rm

#all rule
all: $(PROGC)

$(PROGC): $(OBJSC)
	$(LDP) $^ $(LD_FLAGS) -o $@
%.o: %.c
	$(CC) $(FLAGS) -c $^ -o $@
%.o: %.cpp
	$(CPP) $(FLAGS) -c $^ -o $@

#clean rule
clean:
	$(RM) -rf *.o $(PROGC)
