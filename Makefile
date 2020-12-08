# # # # # # #
# Makefile for Hash Tables
#
# created by Maxim Kirkman <max.kirkman94@gmail.com>
#

CC     = gcc
CFLAGS = -Wall -Wno-format -std=c99
EXE    = ht
OBJ    = src/main.o src/inthash.o src/hashtbl.o src/tables/cuckoo.o \
		 src/tables/xtndbln.o src/tables/xuckoo.o

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

main.o: src/inthash.h src/hashtbl.h
hashtbl.o: src/inthash.h src/tables/cuckoo.h \
  src/tables/xtndbln.h src/tables/xuckoo.h
tables/cuckoo.o: src/inthash.h
tables/xtndbln.o: src/inthash.h
tables/xuckoo.o: src/inthash.h

# CLEANING #
clean:
	rm -f $(OBJ)
clobber: clean
	rm -f $(EXE) 
cleanly: $(EXE) clean