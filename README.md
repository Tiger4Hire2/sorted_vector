# sorted_vector
Proof Big "O" notation leads to the wrong conclussions sometimes

See the open office doc for my ramblings

Building
========

 Requirements
 - build essentials
 - tup build (http://gittup.org/tup/) follow instructions, but bassically run bootstrap.sh
 - googletest (https://github.com/google/googletest), cmake based
 - google benchmark (https://github.com/google/benchmark), cmake based

Once you have the repository and the requirments, you need to initialise tup
> tup init  

then build
> tup

It will build a test application (test) and a timing application (time).
Only the test application has symbols.



Update
======

I switched to comparing three styles of coding.

1) Hand written (intrusive) style code. Used to be very common in games programming.
2) STL container based code.
3) Modern sorted-vector/variant code


--------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations
--------------------------------------------------------------------------
EvalSingleLinkedList                  9183 ns         9182 ns        76230
EvalStandardLib                       9854 ns         9854 ns        71447
EvalModern                            2777 ns         2777 ns       252004
AddRemoveSLL                        266523 ns       266516 ns         2637
AddRemoveStd                      20764413 ns     20764183 ns           32
AddRemoveModern                   18336000 ns     18335467 ns           38
AddRemoveSLLNotStack            7084640026 ns   7084411699 ns            1
AddRemoveStdNotStack              21107791 ns     21106313 ns           33
AddRemoveModernNotStackNotBatch 2196139116 ns   2196084652 ns            1

The results show that the old fashioned intrusive (single) list is still reasonable as an iterative computation.
The intrusive list code is extremely fast when used in stack order (FIFO). It is much easier than most people assume to make code stack-order dependent, but it does require a certain amount of archectual enforcement, which makes it hard to re-engineer older code this way.
Used in the exact wrong way, the insertion/deletion times rocket. The worst case is around 5 order of magnitude slower.

The std::set approach is solid for both iteration and insertion/deletion. This is very much what it is there for, stable performance, not optimimal performance.

The modern style rocks as a computation phase. It is flattered becuase the computations are small and probably the entire code fits into a single uop buffer.
Because we allocate the concrete types in blocks; It is also probably gaining a large advantage from branch prediction.
These advantages are also open to the other implementations, mind, it's just by using simpler building blocks we are giving the CPU a bigger chance of seeing these optimisation possibilities.
Profiling it, you can see it also cache-misses at around half the rate of both the set and single-linked-list.
It is however, vunerable to random-use during insertion and deletion, only making up for that weakness by "batching". Insertion/Block insert/deletes are almost insensitive to the number of elements you add/delete in a single operation. This makes it stable with respect to the worst case.
The modern-style assumes mostly relies on the idea that your code operates in two phases, non-mutating computation amd mutating, which is true for a wide array of applications. This is close to the functional paradigm, of non-mutable data, and works very well in multi-processor environments, where mutating data is dangerous.