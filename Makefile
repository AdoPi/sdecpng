#simple makefile to build sdecpng

sdecpng:
	g++ src/main.cpp -O3  -Wc++11-extensions -lpng -I include -o sdecpng

debug:
	g++ src/main.cpp -g -I include -lpng -o sdecpng_debug


