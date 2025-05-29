CC=gcc
CFLAGS=-g -Wall -std=c99 -fpic
CLINK=$(CC)
RM=rm -rf
SO=libgrades.so
OBJS=grades.o


libgrades.so: $(OBJS)
	$(CLINK) -shared -o $(SO) $(OBJS) -L. -llinked-list

grades.o: grades.c grades.h linked-list.h
	$(CC) $(CFLAGS) -c grades.c

ourtest: our_test.o grades.o
	$(CLINK) our_test.o grades.o -o ourtest.exe -L. -llinked-list -Wl,-rpath=.

our_test.o: grades.c grades.h linked-list.h
	$(CC) $(CFLAGS) -c our_test.c

clean:
	$(RM) *.o $(SO) ourtest.exe