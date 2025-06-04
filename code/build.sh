mkdir -p ../../build
pushd ../../build
gcc -g -o ace64 ../ace64/code/ace64.c ../ace64/code/cpu.h ../ace64/code/cpu.c -lpthread -Llib -Wall -Wno-write-strings -Wno-unused-variable

chmod +x ace64
popd
