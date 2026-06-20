CC = mpicc
CFLAGS = -Wall -O3
TARGETS = matvec_col_vector matvec_col_resized

all: $(TARGETS)

matvec_col_vector: matvec_col_vector.c
	$(CC) $(CFLAGS) -o matvec_col_vector matvec_col_vector.c

matvec_col_resized: matvec_col_resized.c
	$(CC) $(CFLAGS) -o matvec_col_resized matvec_col_resized.c

clean:
	rm -f $(TARGETS)
