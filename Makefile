#simple makefile to build this work in progress tool


sdecpng:
	g++ src/lodepng.cpp src/main.cpp -ansi -pedantic -Wall -Wextra -O3 -I include -o main

