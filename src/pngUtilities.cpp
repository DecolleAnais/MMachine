#include "pngUtilities.hpp"
#include <assert.h>

void PngUtilities::init(const char* path) {

	// load the png
	file = fopen(path, "rb");
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png, file);
	png_read_info(png, info);

	// get data from the png
	width = png_get_image_width(png, info);
	height = png_get_image_height(png, info);
	color_type = png_get_color_type(png, info);
	bit_depth = png_get_bit_depth(png, info);


	// adjust the png parameters

	if(bit_depth == 16)
		png_set_strip_16(png);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||	color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	// update the infos of the png image
	png_read_update_info(png, info);

	// save the rows in an array
	rows = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(unsigned int y = 0; y < height; y++) {
		rows[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	// read the image
	png_read_image(png, rows);
}

void PngUtilities::readRows() const {
	// scan each pixel
	for(unsigned int y = 0; y < height; y++) {
		png_bytep row = rows[y];
		for(unsigned int x = 0; x < width; x++) {
		  png_bytep px = &(row[x * 4]);
		  // process on the pixel
		  printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
		}
	}
}

unsigned int PngUtilities::getValue(const unsigned int x, const unsigned int y) const {
	assert(x < width && y < height && "Try to access a pixel out the image");
	png_bytep row = rows[y];
	png_bytep px = &(row[x * 4]);
	return px[0];
}

unsigned int PngUtilities::getHeight() const {
	return height;
}

unsigned int PngUtilities::getWidth() const {
	return width;
}

void PngUtilities::free() {
	fclose(file);
}

std::string PngUtilities::toString() const {
	return "width = " + std::to_string(width) + " | height = " + std::to_string(height) + " | color_type = " + std::to_string(color_type) + " | bit_depth = " + std::to_string(bit_depth) + "\n";
}