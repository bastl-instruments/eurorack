rm runTest
rm test.o
rm oscil.o
g++ -c -o test.o -DTESTING=1 -I"../../../arduino-base/libs-bastl/bastl" test.cpp
g++ -c -o oscil.o -DTESTING=1 -I"../../../arduino-base/libs-bastl/bastl"  -I"../../../arduino-base/core" ../multiChannelOscillator.cpp
g++ -o runTest test.o oscil.o
./runTest
