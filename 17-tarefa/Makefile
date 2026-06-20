CC = mpicc
CFLAGS = -Wall -O3
TARGET = matvec

all: $(TARGET)

$(TARGET): matvec.c
	$(CC) $(CFLAGS) -o $(TARGET) matvec.c

clean:
	rm -f $(TARGET)
