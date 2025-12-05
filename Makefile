# Compiler and flags
CC = gcc
MPICC = mpicc
CFLAGS = -Wall -std=gnu99 -g -fopenmp
TUNE = -O2
LIBS = src/matrix.c

# Directories
BIN_DIR = bin
SRC_DIR = src
DATA_DIR = data

# Executable names
SEQ_BIN = $(BIN_DIR)/seq
OMP_BIN = $(BIN_DIR)/omp
THREAD_BIN = $(BIN_DIR)/thread
THREAD2_BIN = $(BIN_DIR)/thread2
MPI_BIN = $(BIN_DIR)/mpi

# Default target
all: sequential omp thread thread2 mpi

# Sequential version
sequential: $(SEQ_BIN)

$(SEQ_BIN): $(SRC_DIR)/sequential.c $(LIBS) | $(BIN_DIR)
	$(CC) $(TUNE) $(CFLAGS) -o $@ $(LIBS) $<

# OpenMP version
omp: $(OMP_BIN)

$(OMP_BIN): $(SRC_DIR)/omp.c $(LIBS) | $(BIN_DIR)
	$(CC) $(TUNE) $(CFLAGS) -o $@ $(LIBS) $<

# Pthreads version (element-wise)
thread: $(THREAD_BIN)

$(THREAD_BIN): $(SRC_DIR)/thread.c $(LIBS) | $(BIN_DIR)
	$(CC) $(TUNE) $(CFLAGS) -pthread -o $@ $(LIBS) $<

# Pthreads version (row-wise) - recommended
thread2: $(THREAD2_BIN)

$(THREAD2_BIN): $(SRC_DIR)/thread2.c $(LIBS) | $(BIN_DIR)
	$(CC) $(TUNE) $(CFLAGS) -pthread -o $@ $(LIBS) $<

# MPI version
mpi: $(MPI_BIN)

$(MPI_BIN): $(SRC_DIR)/mpi.c $(LIBS) | $(BIN_DIR)
	$(MPICC) $(TUNE) $(CFLAGS) -o $@ $(LIBS) $<

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Test with small matrices
test: all
	@echo "Testing with small matrices..."
	@echo "=== Sequential ==="
	./$(SEQ_BIN) $(DATA_DIR)/mat_5_4.txt $(DATA_DIR)/mat_4_5.txt
	@echo "=== OpenMP ==="
	./$(OMP_BIN) $(DATA_DIR)/mat_5_4.txt $(DATA_DIR)/mat_4_5.txt
	@echo "=== Pthreads (row-wise) ==="
	./$(THREAD2_BIN) $(DATA_DIR)/mat_5_4.txt $(DATA_DIR)/mat_4_5.txt
	@echo "=== MPI ==="
	mpirun -n 2 ./$(MPI_BIN) $(DATA_DIR)/mat_5_4.txt $(DATA_DIR)/mat_4_5.txt

# Benchmark with medium matrix
benchmark: all
	@echo "Benchmarking with 500x500 matrices..."
	@echo "=== Sequential ==="
	time ./$(SEQ_BIN) data/matrix500_a.txt data/matrix500_b.txt
	@echo "=== OpenMP ==="
	time ./$(OMP_BIN) data/matrix500_a.txt data/matrix500_b.txt
	@echo "=== Pthreads (row-wise) ==="
	time ./$(THREAD2_BIN) data/matrix500_a.txt data/matrix500_b.txt
	@echo "=== MPI (4 processes) ==="
	time mpirun -n 4 ./$(MPI_BIN) data/matrix500_a.txt data/matrix500_b.txt

# Generate test matrices
generate-matrices:
	@echo "Generating test matrices..."
	python3 random_float_matrix.py 5 4 $(DATA_DIR)/mat_5_4.txt
	python3 random_float_matrix.py 4 5 $(DATA_DIR)/mat_4_5.txt
	python3 random_float_matrix.py 10 10 $(DATA_DIR)/mat_10_10_a.txt
	python3 random_float_matrix.py 10 10 $(DATA_DIR)/mat_10_10_b.txt

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)/*

# Install dependencies (Ubuntu/Debian)
deps-ubuntu:
	sudo apt-get update
	sudo apt-get install -y gcc openmpi-bin libopenmpi-dev python3

# Show help
help:
	@echo "Available targets:"
	@echo "  all          - Build all versions (default)"
	@echo "  sequential   - Build sequential version"
	@echo "  omp          - Build OpenMP version"
	@echo "  thread       - Build pthreads (element-wise) version"
	@echo "  thread2      - Build pthreads (row-wise) version"
	@echo "  mpi          - Build MPI version"
	@echo "  test         - Run tests with small matrices"
	@echo "  benchmark    - Run benchmarks with medium matrices"
	@echo "  generate-matrices - Generate test matrices"
	@echo "  clean        - Remove all binaries"
	@echo "  deps-ubuntu  - Install dependencies on Ubuntu"
	@echo "  help         - Show this help message"

.PHONY: all sequential omp thread thread2 mpi test benchmark generate-matrices clean deps-ubuntu help
