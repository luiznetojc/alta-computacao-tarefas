CC = gcc
SRC = teste-parelelismo.c

all: o0 o2 o3

o0:
	$(CC) -O0 $(SRC) -o programa_O0

o2:
	$(CC) -O2 $(SRC) -o programa_O2

o3:
	$(CC) -O3 $(SRC) -o programa_O3

run_o0:
	./programa_O0
	./programa_O2
	./programa_O3

clean:
	rm -f programa_O0 programa_O2 programa_O3