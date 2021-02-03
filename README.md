# Cache-Controller

PROJECT OVERVIEW:
The goal of this project is to determine the best architecture for a cache controller that interfaces to a
16- bit microprocessor that is used for signal processing. While the microprocessor is general purpose in
design and performs a number of functions, it is desired to speed up certain signal processing functions,
such as those calculating the eigenvalues and eigenvectors of a system.
KEY REQUIREMENTS:
 Your project should seek to allow 4GiB of memory (1 Gi x 32 bit) to be interfaced and the cache
should be limited to 128 KiB in size
 After the program being executed was profiled, the largest hit in performance was seen in functions
called choldc() and cholsl() which operate on a 256x256 symmetric matrix of single-precision real
floating numbers
 Please use the assumption that the miss penalty is 60 ns (for first memory access) and 17ns for
subsequent accesses in the same SDRAM word line and the cache hit time is 1 ns.
 Please calculate the performance as a function of set size (n-way), number of cache lines, block size,
and write strategy (write-back allocate, write-through allocate, and write-through non-allocate).
Determine which 3 configurations result in the best performance. At a minimum, n-way associativity
of 1, 2, 4, 8, and 16 and burst lengths of 1, 2, 4, and 8 should be evaluated for all 3 write strategies
(60 permutations)
