#include "../portab.h"
#include "interpolate8x8.h"

/* function pointers */
INTERPOLATE8X8_PTR interpolate8x8_halfpel_h;
INTERPOLATE8X8_PTR interpolate8x8_halfpel_v;
INTERPOLATE8X8_PTR interpolate8x8_halfpel_hv;


/* dst = interpolate(src) */

void
interpolate8x8_halfpel_h_c(uint8_t * const dst,
						   const uint8_t * const src,
						   const uint32_t stride,
						   const uint32_t rounding)
{
	uint32_t i, j;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {

			int32_t tot =
				(int32_t) src[j * stride + i] + (int32_t) src[j * stride + i + 1];

			tot = (tot + 1 - rounding) >> 1;
			dst[j * stride + i] = (uint8_t) tot;
		}
	}
}

// it only calcute 17*17 times for one MV mode. it use for eight point search region.
// by gary
void
OneMV_interpolate8x8_halfpel_h(uint8_t * const dst,
						       const uint8_t * const src,
						       const uint32_t stride,
						       const uint32_t rounding)
{
	uint32_t i, j;

	for (j=0; j<17; j++){
		for (i=0; i<17; i++){
			int32_t tot = (int32_t) src[j * stride + i] + (int32_t) src[j * stride + i + 1];

			tot = (tot + 1 - rounding) >> 1;
			dst[j*17 + i] = (uint8_t) tot;
		}
	}
}



void
interpolate8x8_halfpel_v_c(uint8_t * const dst,
						   const uint8_t * const src,
						   const uint32_t stride,
						   const uint32_t rounding)
{
	uint32_t i, j;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			int32_t tot = 
				(int32_t)src[j * stride + i] + (int32_t)src[j * stride + i + stride];

			tot = ((tot + 1 - rounding) >> 1);
			dst[j * stride + i] = (uint8_t) tot;
		}
	}
}

// it only calcute 17*17 times for one MV mode. it use for eight point search region.
// by gary

void
OneMV_interpolate8x8_halfpel_v(uint8_t * const dst,
						       const uint8_t * const src,
						       const uint32_t stride,
						       const uint32_t rounding)
{
	uint32_t i, j;

	for (j=0 ; j<17 ; j++){
		for (i=0 ; i<17 ; i++){
			int32_t tot = (int32_t)src[j * stride + i] + (int32_t)src[j * stride + i + stride];

			tot = ((tot + 1 - rounding) >> 1);
			dst[j * 17 + i] = (uint8_t) tot;
		}
	}
}


void
interpolate8x8_halfpel_hv_c(uint8_t * const dst,
							const uint8_t * const src,
							const uint32_t stride,
							const uint32_t rounding)
{
	uint32_t i, j;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			int32_t tot =
				(int32_t)src[j * stride + i] + (int32_t)src[j * stride + i + 1] +
				(int32_t)src[j * stride + i + stride] + (int32_t)src[j * stride + i + stride + 1];
			tot = ((tot + 2 - rounding) >> 2);
			dst[j * stride + i] = (uint8_t) tot;
		}
	}
}

// it only calcute 17*17 times for one MV mode. it use for eight point search region.
// by gary

void
OneMV_interpolate8x8_halfpel_hv(uint8_t * const dst,
							    const uint8_t * const src,
						    	const uint32_t stride,
						    	const uint32_t rounding)
{
	uint32_t i, j;

	for (j = 0; j < 17; j++) {
		for (i = 0; i < 17; i++) {
			int32_t tot =
				(int32_t)src[j * stride + i] + (int32_t)src[j * stride + i + 1] +
				(int32_t)src[j * stride + i + stride] + (int32_t)src[j * stride + i + stride + 1];
			tot = ((tot + 2 - rounding) >> 2);
			dst[j * 17 + i] = (uint8_t) tot;
		}
	}
}


/* add by MinChen <chenm001@163.com> */
/* interpolate8x8 two pred block */
void
interpolate8x8_c(uint8_t * const dst,
				 const uint8_t * const src,
				 const uint32_t x,
				 const uint32_t y,
				 const uint32_t stride)
{
	uint32_t i, j;

	for (j = 0; j < 8; j++) {
		for (i = 0; i < 8; i++) {
			int32_t tot =
				(((int32_t)src[(y + j) * stride + x + i] +
				  (int32_t)dst[(y + j) * stride + x + i] + 1) >> 1);
			dst[(y + j) * stride + x + i] = (uint8_t) tot;
		}
	}
}
