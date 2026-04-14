OFILES_testpdlist = testcputime.o PointDctList.o Point.o List.o
OFILES_testpdbst = testcputime.o PointDctBST.o Point.o List.o BST.o
OFILES_testpdbst2d = testcputime.o PointDctBST2d.o Point.o List.o
OFILES_taxi = testtaxi.o PointDctList.o Point.o List.o

TARGET_testpdlist = testpdlist
TARGET_testpdbst = testpdbst
TARGET_testpdbst2d = testpdbst2d
TARGET_taxi = testtaxi

CC = gcc
CFLAGS = -Wall -Wextra -Wmissing-prototypes --pedantic -std=c99

.PHONY: all clean run archive

LDFLAGS = -lm

all: $(TARGET_testpdlist) $(TARGET_testpdbst) $(TARGET_testpdbst2d) $(TARGET_taxi)
clean:
	rm -f $(OFILES_testpdlist) $(OFILES_testpdbst) $(OFILES_testpdbst2d) $(OFILES_taxi) $(OFILES_testbst) $(OFILES_testbst2d)
	rm -f $(TARGET_testpdlist) $(TARGET_testpdbst) $(TARGET_testpdbst2d) $(TARGET_taxi)
run: $(TARGET_testpdlist) $(TARGET_testpdbst) $(TARGET_testpdbst2d)
	./$(TARGET_testpdlist) 1000000 10000 0.01
	./$(TARGET_testpdbst) 1000000 10000 0.01
	./$(TARGET_testpdbst2d) 1000000 10000 0.01
archive:
	zip fichiers-p2-2025-2026.zip BST.c BST.h declaration.txt List.c List.h Makefile Point.c Point.h PointDct.h PointDctBST.c PointDctBST2d.c PointDctList.c testcputime.c testtaxi.c
$(TARGET_testpdlist): $(OFILES_testpdlist)
	$(CC) -o $(TARGET_testpdlist) $(OFILES_testpdlist) $(LDFLAGS)
$(TARGET_testpdbst): $(OFILES_testpdbst)
	$(CC) -o $(TARGET_testpdbst) $(OFILES_testpdbst) $(LDFLAGS)
$(TARGET_testpdbst2d): $(OFILES_testpdbst2d)
	$(CC) -o $(TARGET_testpdbst2d) $(OFILES_testpdbst2d) $(LDFLAGS)
$(TARGET_taxi): $(OFILES_taxi)
	$(CC) -o $(TARGET_taxi) $(OFILES_taxi) $(LDFLAGS)

BST.o: BST.c BST.h List.h
List.o: List.c List.h
Point.o: Point.c Point.h
PointDctBST.o: PointDctBST.c PointDct.h List.h Point.h BST.h
PointDctBST2d.o: PointDctBST2d.c PointDct.h List.h Point.h
PointDctList.o: PointDctList.c PointDct.h List.h Point.h
testcputime.o: testcputime.c PointDct.h List.h Point.h
testtaxi.o: testtaxi.c PointDct.h List.h Point.h

