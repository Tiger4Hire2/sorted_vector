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


Results
=======

Based off my lowly laptop, but 
