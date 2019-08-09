CPP=mpicxx
LDP=mpicxx
LD_FLAGS = -fopenmp -lstdc++ -lopencv_core -lopencv_highgui -lopencv_imgproc -g
FLAGS= -fopenmp -I/usr/include/opencv -g
PROGC = mW.x
OBJSC = mW.o videoLoader.o convolution.o

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
