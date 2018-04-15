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

static const int kPNGHeaderSize = 8;

bool is_png(std::istream& source) {

	png_byte header[kPNGHeaderSize];

	// read the bytes corresponding to the png header
	source.read((char*)header, kPNGHeaderSize);
	if (!source.good()) return false;

	// check PNG header
	return (png_sig_cmp(header, 0, kPNGHeaderSize) == 0);
}


void source_reader(png_structp currentPng, png_bytep data, png_size_t length) {
	((std::istream*) png_get_io_ptr(currentPng))->read((char*)data, length);
}

int main() {

	int frames = 0;
	// Looper
	while(true) {

		// validate if we have a correct png file
		if (!is_png(std::cin)) {
			if (!frames) {
				std::cerr << "Err: No valid input" << std::endl;
				return 1;
			}
			return 0;
		}

		// init png structure
		png_structp currentPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!currentPng) {
			std::cerr << "Err: Can't initialize PNG structure" << std::endl;
			return 2;
		}

		//Here we create the png info struct.
		//Note that this time, if this function fails, we have to clean up the read struct!
		png_infop pngInfo = png_create_info_struct(currentPng);
		if (!pngInfo) {
			std::cerr << "Err: Can't initialize PNG structure" << std::endl;
			png_destroy_read_struct(&currentPng, (png_infopp)0, (png_infopp)0);
			return 2;
		}


		// reader from std::cin
		png_set_read_fn(currentPng,(png_voidp)&std::cin, source_reader);
		png_set_sig_bytes(currentPng, kPNGHeaderSize);

		// get png information
		png_read_info(currentPng, pngInfo);

		png_uint_32 imgWidth =  png_get_image_width(currentPng, pngInfo);
		png_uint_32 imgHeight = png_get_image_height(currentPng, pngInfo);
		png_uint_32 bitdepth   = png_get_bit_depth(currentPng, pngInfo);
		png_uint_32 channels   = png_get_channels(currentPng, pngInfo);
		//png_uint_32 color_type = png_get_color_type(currentPng, pngInfo);


		std::cout << "width=" << imgWidth << ", height=" << imgHeight << std::endl;

		// Read png
		png_bytep* rowPtrs = new png_bytep[imgHeight];
		int dataSize = imgWidth * imgHeight * bitdepth * channels / 8;
		char* data = new char[dataSize];

		const unsigned int stride = imgWidth * bitdepth * channels / 8;
		for (size_t i = 0; i < imgHeight; i++) {
			png_uint_32 q = (imgHeight- i - 1) * stride;
			rowPtrs[i] = (png_bytep)data + q;
		}

		//read png into data
		png_read_image(currentPng, rowPtrs);

		//here write raw image
		std::string filename;
		filename << "image" << frames << ".rgb24";
		std::ofstream outfile (filename,std::ofstream::binary);
		outfile.write (data,dataSize);

		outfile.close();


    // read end of png
    png_read_end(currentPng, pngInfo);

		// clean up
		delete[] (png_bytep)rowPtrs;
		png_destroy_read_struct(&currentPng, &pngInfo,(png_infopp)0);

		frames += 1;

	}

	return 0;
}

