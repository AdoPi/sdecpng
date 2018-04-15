#simple makefile to build this work in progress tool

sdecpng:
	g++ src/main.cpp -O3  -Wc++11-extensions -lpng -I include -o main

debug:
	g++ src/main.cpp -g -I include -o main


