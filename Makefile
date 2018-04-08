#simple makefile to build this work in progress tool


sdecpng:
	g++ src/lodepng.cpp src/main.cpp -ansi -pedantic -Wall -Wextra -O3  -Wc++11-extensions -I include -o main

debug:
	g++ src/lodepng.cpp src/main.cpp -ansi -pedantic -Wall -Wextra -g -I include -o main


