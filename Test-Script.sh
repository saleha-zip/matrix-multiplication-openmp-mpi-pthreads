#!/bin/bash

echo "Generate test matrices with Python if no test data found"
echo

# Create data directory if it doesn't exist
mkdir -p data

# Generate matrices for reasonable benchmark sizes
sizes=( "5 4" "4 5" "10 10" "100 100" "200 200" "500 500" )

for size in "${sizes[@]}"; do
    rows=$(echo $size | cut -d' ' -f1)
    cols=$(echo $size | cut -d' ' -f2)

    file_a="data/mat_${rows}x${cols}a.txt"
    file_b="data/mat_${rows}x${cols}b.txt"

    if [ ! -f "$file_a" ]; then
        echo "Generating ${rows}x${cols} matrix A..."
        python3 random_float_matrix.py $rows $cols "$file_a"
    fi

    if [ ! -f "$file_b" ]; then
        echo "Generating ${rows}x${cols} matrix B..."
        python3 random_float_matrix.py $cols $rows "$file_b"  # Note: cols x rows for valid multiplication
    fi
done

echo "Compiling all versions..."
echo
make

echo
echo "Benchmarking different implementations..."
echo

# Test matrices to benchmark (reasonable sizes)
test_sizes=("10x10" "100x100" "200x200" "500x500")

for size in "${test_sizes[@]}"; do
    echo "* * * * * * * ${size} Matrix Multiplication"
    echo

    matrix_a="data/mat_${size}a.txt"
    matrix_b="data/mat_${size}b.txt"

    # Check if files exist and have content
    if [ ! -f "$matrix_a" ] || [ ! -f "$matrix_b" ] || [ ! -s "$matrix_a" ] || [ ! -s "$matrix_b" ]; then
        echo "Warning: Test files for ${size} not found or empty, skipping..."
        echo
        continue
    fi

    echo "Sequential:"
    bin/seq "$matrix_a" "$matrix_b" 2>/dev/null | grep -E "(Time:|Matrix Multiplication:)"

    echo "OpenMP:"
    bin/omp "$matrix_a" "$matrix_b" 2>/dev/null | grep -E "(Time:|Matrix Multiplication:|threads)"

    echo "Pthreads:"
    bin/thread2 "$matrix_a" "$matrix_b" 2>/dev/null | grep -E "(Time:|Matrix Multiplication:|threads)"

    echo "MPI (4 processes):"
    mpirun -np 4 bin/mpi "$matrix_a" "$matrix_b" 2>/dev/null | grep -E "(Time:|Matrix Multiplication:)"

    echo
    echo "---"
    echo
done

# Small matrix test to verify correctness
echo "Correctness test with 5x4 and 4x5 matrices:"
echo "Sequential:"
bin/seq data/mat_5x4a.txt data/mat_4x5b.txt | tail -n 7
echo "OpenMP:"
bin/omp data/mat_5x4a.txt data/mat_4x5b.txt | tail -n 7
echo "Pthreads:"
bin/thread2 data/mat_5x4a.txt data/mat_4x5b.txt | tail -n 7
echo "MPI:"
mpirun -np 2 bin/mpi data/mat_5x4a.txt data/mat_4x5b.txt | tail -n 7
echo "All results above should be identical"
echo
