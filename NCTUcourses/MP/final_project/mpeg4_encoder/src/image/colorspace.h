#ifndef _COLORSPACE_H

#define _COLORSPACE_H

#include "../portab.h"

/* input color conversion functions (encoder) */

typedef void (color_inputFunc) (uint8_t * y_out,
								uint8_t * u_out,
								uint8_t * v_out,
								uint8_t * src,
								int width,
								int height,
								int stride);

typedef color_inputFunc *color_inputFuncPtr;

extern color_inputFuncPtr yuv_to_yv12;

/* plain c */
color_inputFunc yuv_to_yv12_c;



#endif							/* _COLORSPACE_H_ */
