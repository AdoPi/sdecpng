#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <iterator>
#include <vector>
#include <algorithm>
#include <array>

#include "png.h"


//png checker
#define PNGSIGSIZE 8

bool validate(std::istream& source) {

	//Allocate a buffer of 8 bytes, where we can put the file signature.
	png_byte pngsig[PNGSIGSIZE];
	int is_png = 0;

	//Read the 8 bytes from the stream into the sig buffer.
	source.read((char*)pngsig, PNGSIGSIZE);

	//Check if the read worked...
	if (!source.good()) return false;

	//Let LibPNG check the sig. If this function returns 0, everything is OK.
	is_png = png_sig_cmp(pngsig, 0, PNGSIGSIZE);
	return (is_png == 0);
}


// thread here ?
void userReadData(png_structp pngPtr, png_bytep data, png_size_t length) {
    //Here we get our IO pointer back from the read struct.
    //This is the parameter we passed to the png_set_read_fn() function.
    //Our std::istream pointer.
    png_voidp a = png_get_io_ptr(pngPtr);
    //Cast the pointer to std::istream* and read 'length' bytes into 'data'
    ((std::istream*)a)->read((char*)data, length);
}

int main() {

	//so First, we validate our stream with the validate function I just mentioned
	if (!validate(std::cin)) {
		std::cerr << "ERROR: Data is not valid PNG-data" << std::endl;
		return -1; //Do your own error recovery/handling here
	}

	//Here we create the png read struct. The 3 NULL's at the end can be used
	//for your own custom error handling functions, but we'll just use the default.
	//if the function fails, NULL is returned. Always check the return values!
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngPtr) {
		std::cerr << "ERROR: Couldn't initialize png read struct" << std::endl;
		return false; //Do your own error recovery/handling here
	}

	//Here we create the png info struct.
	//Note that this time, if this function fails, we have to clean up the read struct!
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		std::cerr << "ERROR: Couldn't initialize png info struct" << std::endl;
		png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
		return false; //Do your own error recovery/handling here
	}


	// read from std::cin
	png_set_read_fn(pngPtr,(png_voidp)&std::cin, userReadData);

	//Set the amount signature bytes we've already read:
    //We've defined PNGSIGSIZE as 8;
    png_set_sig_bytes(pngPtr, PNGSIGSIZE);

    //Now call png_read_info with our pngPtr as image handle, and infoPtr to receive the file info.
    png_read_info(pngPtr, infoPtr);

	//read some data from the png
    png_uint_32 imgWidth =  png_get_image_width(pngPtr, infoPtr);
    png_uint_32 imgHeight = png_get_image_height(pngPtr, infoPtr);

    //bits per CHANNEL! note: not per pixel!
    png_uint_32 bitdepth   = png_get_bit_depth(pngPtr, infoPtr);
    //Number of channels
    png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);
    //Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);


	printf("width=%d, height=%d\n",imgWidth,imgHeight);


	// READING BITMAP



	//Here's one of the pointers we've defined in the error handler section:
    //Array of row pointers. One for every row.
    png_bytep* rowPtrs = new png_bytep[imgHeight];

    //Alocate a buffer with enough space.
    //(Don't use the stack, these blocks get big easilly)
    //This pointer was also defined in the error handling section, so we can clean it up on error.
	int dataSize = imgWidth * imgHeight * bitdepth * channels / 8;
    char* data = new char[dataSize];
    //This is the length in bytes, of one row.
    const unsigned int stride = imgWidth * bitdepth * channels / 8;

    //A little for-loop here to set all the row pointers to the starting
    //Adresses for every row in the buffer

    for (size_t i = 0; i < imgHeight; i++) {
        //Set the pointer to the data pointer + i times the row stride.
        //Notice that the row order is reversed with q.
        //This is how at least OpenGL expects it,
        //and how many other image loaders present the data.
        png_uint_32 q = (imgHeight- i - 1) * stride;
        rowPtrs[i] = (png_bytep)data + q;
	}

    //And here it is! The actuall reading of the image!
    //Read the imagedata and write it to the adresses pointed to
    //by rowptrs (in other words: our image databuffer)
    png_read_image(pngPtr, rowPtrs);


	//here write raw image

	std::ofstream outfile ("image.rgb24",std::ofstream::binary);

  // write to outfile
  outfile.write (data,dataSize);



	return 0;
}

