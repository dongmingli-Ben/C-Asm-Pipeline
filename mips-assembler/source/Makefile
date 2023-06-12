# makefile template
# 
# Special Symbols used:
# 	$@ name of the file to be made (target)
# 	$* prefix of the file to be made (target)
# 	$? names of the CHANGED dependents.
#	$^ names of ALL the dependents (dependency).
# 	$< name of the related file that caused the action (command)
#
# Usage:
#	make all
#		It compiles everything so that you can do local testing 
#		before installing applications.
#
#	make clean
#		It cleans applications, gets rid of the executables, 
#		any temporary files, object files, etc.
#
#	make echo
#		It echo a string to the console.
#
# Custom Macros:
#
#	PROGRAM
#		Name of the executable program used to test your assignment
#
#	OBJECTS
#		Names of all the object files needed to be linked
#
# Assumptions:
#
#	If header file is used, it must share the SAME filename as
#	the source file.  Ex: Assignment1.cpp, Assignment1.h
#
#
# Note:
#	C++17 version is used to compile the source files.
#
#	lib.cpp contains the functions extracted from the stanford library.
#	
# Revision History:
#	2022/Sept/04	Initial Creation	Kinley	
#   2022/Sept/22    modified            Rulan


PROGRAM = \
	run \
	debug 


CPPFLAGS = -std=c++17
CC = g++

all: run

run: tester.o phase1.o phase2.o utils.o
	$(CC) $(CPPFLAGS) -o $@ $^

tester.o: tester.c 
	$(CC) $(CPPFLAGS) -c $^ -o $@

phase1.o: phase1.cpp 
	$(CC) $(CPPFLAGS) -c $^ -o $@

phase2.o: phase2.cpp 
	$(CC) $(CPPFLAGS) -c $^ -o $@

utils.o: utils.cpp 
	$(CC) $(CPPFLAGS) -c $^ -o $@

debug:
	$(CC) $(CPPFLAGS) -g tester.c phase1.cpp phase2.cpp utils.cpp -o debug

test:
	make run
	./run testfile.asm output.txt expectedoutput.txt 
	./run testfile2.asm output.txt expectedoutput2.txt 
	./run testfile3.asm output.txt expectedoutput3.txt 


# ***************************************************************
# Standard entries to remove files from the directories
#    clean -- delete derived files in preparation for rebuild

clean:
	rm -f *.o *.a $(PROGRAM)
