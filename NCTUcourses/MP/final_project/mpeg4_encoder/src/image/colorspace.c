//#include <string.h>				/* memcpy */
#include "../xvid.h"

#include "colorspace.h"

/* function pointers */

/* input */
color_inputFuncPtr yuv_to_yv12;


/*	yuv planar -> yuv 4:2:0 planar
   
	NOTE: does not flip */

void
yuv_to_yv12_c(uint8_t * y_out,
			  uint8_t * u_out,
			  uint8_t * v_out,
			  uint8_t * src,
			  int width,
			  int height,
			  int stride)
{
	uint32_t stride2 = stride >> 1;
	uint32_t width2 = width >> 1;
	uint32_t y;

	for (y = height; y; y--) {
		my_memcpy(y_out, src, width);
		src += width;
		y_out += stride;
	}

	for (y = height >> 1; y; y--) {
		my_memcpy(u_out, src, width2);
		src += width2;
		u_out += stride2;
	}

	for (y = height >> 1; y; y--) {
		my_memcpy(v_out, src, width2);
		src += width2;
		v_out += stride2;
	}
}


