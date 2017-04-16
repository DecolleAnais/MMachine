#ifndef _MMACHINE_PNG_UTILITIES_HPP_
#define _MMACHINE_PNG_UTILITIES_HPP_

#include "png.h"
#include "zlib.h"
#include <string>

class PngUtilities {
	public :
		void init(const char* path);
		void free();

		void readRows() const;
		unsigned int getValue(const unsigned int x, const unsigned int y) const;
		unsigned int getHeight() const;
		unsigned int getWidth() const;

		std::string toString() const;

	private :
		FILE *file;
		unsigned int width;
		unsigned int height;
		png_byte color_type;
		png_byte bit_depth;
		png_bytep *rows;
};

#endif