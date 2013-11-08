#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "../portab.h"
#include "colorspace.h"
#include "../xvid.h"

#define EDGE_SIZE  32


typedef struct
{
	uint8_t *y;
	uint8_t *u;
	uint8_t *v;
}
IMAGE;

void init_image(uint32_t cpu_flags);

int32_t image_create(IMAGE * image,
					 uint32_t edged_width,
					 uint32_t edged_height);
void image_destroy(IMAGE * image,
				   uint32_t edged_width,
				   uint32_t edged_height);

void image_copy(IMAGE * image1,
				IMAGE * image2,
				uint32_t edged_width,
				uint32_t height);

void image_setedges(IMAGE * image,
					uint32_t edged_width,
					uint32_t edged_height,
					uint32_t width,
					uint32_t height);

void image_interpolate(const IMAGE * refn,
					   IMAGE * refh,
					   IMAGE * refv,
					   IMAGE * refhv,
					   uint32_t edged_width,
					   uint32_t edged_height,
					   uint32_t rounding);

//Parallel code .by gary
void
OneMV_image_interpolate(const IMAGE * refn,
						int32_t x,
						int32_t y,
				        IMAGE * refh,
				        IMAGE * refv,
				        IMAGE * refhv,
				        uint32_t edged_width,
				        uint32_t edged_height,
				        uint32_t rounding);
#define _MSC_VER
#if defined(_MSC_VER)
float image_psnr(IMAGE * orig_image,
				 IMAGE * recon_image,
				 uint16_t stride,
				 uint16_t width,
				 uint16_t height);
#endif

#endif							/* _IMAGE_H_ */
