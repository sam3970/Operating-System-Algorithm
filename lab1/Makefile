#
#	DKU Operating System Lab
#	    Lab1 (Scheduler Algorithm Simulator)
#	    Student id : 
#	    Student name : 
#
#   Makfeile :
#       - Makefile for lab1 compilation.
#

CC = g++
INC = -I${CURDIR}/include/
CFLAGS = -g $(INC)

OBJS_LAB1 = lab1_sched.o lab1_sched_test.o 

SRCS = $(OBJS_LAB1:.o=.cpp)

TARGET_LAB1 = lab1_sched

.SUFFIXES : .cpp .o

.cpp.o:
	@echo "Compiling lab1 scheduler simulator $< ..."
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET_LAB1) : $(OBJS_LAB1)
	$(CC) -o $(TARGET_LAB1) $(OBJS_LAB1)

all : $(TARGET_LAB1)

dep : 
	$(CC) -M $(INC) $(SRCS)

clean :
	@echo "Cleaning lab1 scheduler simulator $< ..."
	rm -rf $(OBJS_LAB1) $(TARGET_LAB1) 

new :
	$(MAKE) clean
	$(MAKE)
