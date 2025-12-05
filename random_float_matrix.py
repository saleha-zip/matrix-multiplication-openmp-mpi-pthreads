#!/usr/bin/env python3

import sys
import random

if len(sys.argv) != 4:
    print("Usage: python3 random_float_matrix.py rows cols output_file")
    print("Output: Generates a matrix with random float values to stdout")
    sys.exit(1)

rows = int(sys.argv[1])
cols = int(sys.argv[2])
output_file = sys.argv[3]

with open(output_file, 'w') as f:
    for i in range(rows):
        row = [f"{random.uniform(0, 100):.6f}" for j in range(cols)]
        f.write(" ".join(row) + "\n")

print(f"Generated {rows}x{cols} matrix in {output_file}")
