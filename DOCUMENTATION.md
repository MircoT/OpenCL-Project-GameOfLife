## Description of the problem

The main purpose of this project was to measure performance improvements when using parallel programming.
This approach can be used to improve all kinds of cellular automata, but I decided to focus on Game Of Life only.

## The program

The simulator creates for the automata a fixed dimension environment , a simple 1D-vector.
This vector, however, reproduces a 2D, toroidal shaped, closed space.

Each cell of the matrix has a Moore neighbourhood; the automata uses this information to compute the transition function on the cells. Update must be performed at the same time on each cell, i.e. no cell can jump to state *t+1* before all other cells reach time *t*.

The algorithm is composed of three steps:

* Creation of a copy of the current matrix, but containing empty cells
* For each cell, calculation of the new state and memorization in the new matrix
* Copy of the results and deletion of old matrix

First step is mandatory, in order to avoid loss of information about the current state by overwritting the current matrix.

Step n°2 is the heaviest of the three. When executing the serial algorithm, cell number *n* must wait for all the previous cells to complete their transition, whilst in the parallel algorithm each cell may switch to the new state independently, without increasing memory usage.

## Benchmarks 

During tests, a single passage of the algorithm was executed on different-size matrices.
The figures show the duration of the execution, in milliseconds.

Tests were performed on an *Intel(R) Core(TM) i5 - 4258U CPU @ 2.40GHz* platform.
The parallel algorithm could access the *280 compute units* of the *Intel Iris 5100*.
*OpenCL* version used is *1.2*; the same for the *OpenCL C compiler*.

| **Matrix dimension** | **Serial algorithm** | **Parallel implementation** |
| --------------------:| --------------------:|:--------------------------- |
| 128 x 128            | 4                    | 1                           |
| 256 x 256            | 9                    | 1                           |
| 512 x 512            | 35                   | 7                           |
| 1024 x 1024          | 136                  | 23                          |
| 2048 x 2048          | 545                  | 86                          |
| 4096 x 4096          | 2171                 | 336                         |
| 8192 x 8129          | 8842                 | 1015                        |
| 10000 x 10000        | 13313                | 1552                        |

## Conclusions and considerations

Parallel implementation of a synchronous automaton leads to a significant performance increase, this is because the transition functions of individual cells can be calculated in a competitive and non-sequential way.

Further studies may relate on the specialization of the OpenCL kernel, probably using also worker groups that can be useful for a specific automata.
