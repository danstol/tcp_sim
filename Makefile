CC = gcc	
CFLAGS = 

PROG = ./a3

HDRS = window.h linked_list.h UpperLayer.h
SRCS = a3.c 
OBJS = a3.o 

# WARNING: *must* have a tab before each definition
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

a3.o: a3.c $(HDRS)
	$(CC) $(CFLAGS) -c a3.c -o a3.o

#UpperLayer.o: UpperLayer.cpp $(HDRS)
#	$(CC) $(CFLAGS) -c UpperLayer.cpp -o UpperLayer.o

# rules for other targets (utility functions)

clean:
	rm -f $(PROG) $(OBJS)
