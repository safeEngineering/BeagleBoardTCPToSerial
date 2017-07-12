# BeagleBoardTCPToSerial

Use "git pull origin master" in the src directory of the project to get the latest files.
Use "sftp debian@10.x.x.x" in the Debug directory of the project to transfer the Test2 file to the BeagleBone \tmp directory using "cd /tmp" and "put Test2"

Test2.cpp - is a test program to test the ASIO library is compiling OK and working using a simple Serial Port Loop back test on TXD1 (P9-24 pinout) and RXD1 (P9-26 pinout) on Uart 1 (/dev/ttyO1) 

Source Code was compiled under Eclipse using ARM Cross Compiler
gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin


Note on compilation - I had to exclude the file "/asio-1.10.6/include/asio/impl/src.cpp" from the project build to get the project to compile.

Also had to use the following Compiler G++ options:
-std=c++0x -DASIO_STANDALONE -DASIO_HAS_STD_ADDRESSOF -I"/home/adrian/workspace/Test2/src/asio-1.10.6/include" -O0 -g3 -Wall -c -fmessage-length=0

And following Linker G++ options:
-lpthread






