#!/bin/bash
g++ -I/home/nvidia/darknet/include/ -I/home/nvidia/darknet/src/ -DOPENCV -I/usr/local/include/opencv -I/usr/local/include  -DGPU -I/usr/local/cuda/include/ -DCUDNN  -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC  -Ofast -DOPENCV -DGPU -DCUDNN -c vision.cpp -o vision.o
gcc -I/home/nvidia/darknet/include/ -I/home/nvidia/darknet/src/ -DOPENCV `pkg-config --cflags opencv`  -DGPU -I/usr/local/cuda/include/ -DCUDNN  -Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC  -Ofast -DOPENCV -DGPU -DCUDNN vision.o libdarknet.a -o vision -lm -pthread  `pkg-config --libs opencv` -lstdc++ -L/usr/local/cuda/lib64 -lcuda -lcudart -lcublas -lcurand -lcudnn -lstdc++  libdarknet.a
rm vision.o
