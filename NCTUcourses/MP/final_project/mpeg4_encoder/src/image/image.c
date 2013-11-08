#include "../portab.h"
#include "../xvid.h"			/* XVID_CSP_XXX's */
#include "image.h"
#include "colorspace.h"
#include "interpolate8x8.h"
#include "../utils/mem_align.h"

#define SAFETY	64
#define EDGE_SIZE2  (EDGE_SIZE/2)


int32_t
image_create(IMAGE * image,
			 uint32_t edged_width,
			 uint32_t edged_height)
{
	const uint32_t edged_width2 = edged_width / 2;
	const uint32_t edged_height2 = edged_height / 2;
	uint32_t i;

	image->y =
		xvid_malloc(edged_width * (edged_height + 1) + SAFETY, CACHE_LINE);
	if (image->y == NULL) {
		return -1;
	}

	for (i = 0; i < edged_width * edged_height + SAFETY; i++) {
		image->y[i] = 0;
	}

	image->u = xvid_malloc(edged_width2 * edged_height2 + SAFETY, CACHE_LINE);
	if (image->u == NULL) {
		xvid_free(image->y);
		return -1;
	}
	image->v = xvid_malloc(edged_width2 * edged_height2 + SAFETY, CACHE_LINE);
	if (image->v == NULL) {
		xvid_free(image->u);
		xvid_free(image->y);
		return -1;
	}

	image->y += EDGE_SIZE * edged_width + EDGE_SIZE;
	image->u += EDGE_SIZE2 * edged_width2 + EDGE_SIZE2;
	image->v += EDGE_SIZE2 * edged_width2 + EDGE_SIZE2;

	return 0;
}



void
image_destroy(IMAGE * image,
			  uint32_t edged_width,
			  uint32_t edged_height)
{
	const uint32_t edged_width2 = edged_width / 2;

	if (image->y) {
		xvid_free(image->y - (EDGE_SIZE * edged_width + EDGE_SIZE));
	}
	if (image->u) {
		xvid_free(image->u - (EDGE_SIZE2 * edged_width2 + EDGE_SIZE2));
	}
	if (image->v) {
		xvid_free(image->v - (EDGE_SIZE2 * edged_width2 + EDGE_SIZE2));
	}
}


void
image_copy(IMAGE * image1,
		   IMAGE * image2,
		   uint32_t edged_width,
		   uint32_t height)
{
	my_memcpy(image1->y, image2->y, edged_width * height);
	my_memcpy(image1->u, image2->u, edged_width * height / 4);
	my_memcpy(image1->v, image2->v, edged_width * height / 4);
}


void
image_setedges(IMAGE * image,
			   uint32_t edged_width,
			   uint32_t edged_height,
			   uint32_t width,
			   uint32_t height)
{
	const uint32_t edged_width2 = edged_width / 2;
	const uint32_t width2 = width / 2;
	uint32_t i;
	uint8_t *dst;
	uint8_t *src;


	dst = image->y - (EDGE_SIZE + EDGE_SIZE * edged_width);
	src = image->y;

	for (i = 0; i < EDGE_SIZE; i++) {
			my_memset(dst, *src, EDGE_SIZE);
			my_memcpy(dst + EDGE_SIZE, src, width);
			my_memset(dst + edged_width - EDGE_SIZE, *(src + width - 1),
				   EDGE_SIZE);
		dst += edged_width;
	}

	for (i = 0; i < height; i++) {
		my_memset(dst, *src, EDGE_SIZE);
		my_memset(dst + edged_width - EDGE_SIZE, src[width - 1], EDGE_SIZE);
		dst += edged_width;
		src += edged_width;
	}

	src -= edged_width;
	for (i = 0; i < EDGE_SIZE; i++) {
			my_memset(dst, *src, EDGE_SIZE);
			my_memcpy(dst + EDGE_SIZE, src, width);
			my_memset(dst + edged_width - EDGE_SIZE, *(src + width - 1),
				   EDGE_SIZE);
		dst += edged_width;
	}


/*U */
	dst = image->u - (EDGE_SIZE2 + EDGE_SIZE2 * edged_width2);
	src = image->u;

	for (i = 0; i < EDGE_SIZE2; i++) {
		my_memset(dst, *src, EDGE_SIZE2);
		my_memcpy(dst + EDGE_SIZE2, src, width2);
		my_memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}

	for (i = 0; i < height / 2; i++) {
		my_memset(dst, *src, EDGE_SIZE2);
		my_memset(dst + edged_width2 - EDGE_SIZE2, src[width2 - 1], EDGE_SIZE2);
		dst += edged_width2;
		src += edged_width2;
	}
	src -= edged_width2;
	for (i = 0; i < EDGE_SIZE2; i++) {
		my_memset(dst, *src, EDGE_SIZE2);
		my_memcpy(dst + EDGE_SIZE2, src, width2);
		my_memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}


/* V */
	dst = image->v - (EDGE_SIZE2 + EDGE_SIZE2 * edged_width2);
	src = image->v;

	for (i = 0; i < EDGE_SIZE2; i++) {
		my_memset(dst, *src, EDGE_SIZE2);
		my_memcpy(dst + EDGE_SIZE2, src, width2);
		my_memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}

	for (i = 0; i < height / 2; i++) {
		my_memset(dst, *src, EDGE_SIZE2);
		my_memset(dst + edged_width2 - EDGE_SIZE2, src[width2 - 1], EDGE_SIZE2);
		dst += edged_width2;
		src += edged_width2;
	}
	src -= edged_width2;
	for (i = 0; i < EDGE_SIZE2; i++) {
		my_memset(dst, *src, EDGE_SIZE2);
		my_memcpy(dst + EDGE_SIZE2, src, width2);
		my_memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}
}

/* bframe encoding requires image-based u,v interpolation */
void
image_interpolate(const IMAGE * refn,
				  IMAGE * refh,
				  IMAGE * refv,
				  IMAGE * refhv,
				  uint32_t edged_width,
				  uint32_t edged_height,
				  uint32_t rounding)
{
	const uint32_t offset = EDGE_SIZE * (edged_width + 1);
	const uint32_t stride_add = 7 * edged_width;

	uint8_t *n_ptr, *h_ptr, *v_ptr, *hv_ptr;
	uint32_t x, y;


	n_ptr = refn->y;
	h_ptr = refh->y;
	v_ptr = refv->y;
	hv_ptr = refhv->y;

	n_ptr -= offset;
	h_ptr -= offset;
	v_ptr -= offset;
	hv_ptr -= offset;

	for (y = 0; y < edged_height; y = y + 8) {
		for (x = 0; x < edged_width; x = x + 8) {
			interpolate8x8_halfpel_h(h_ptr, n_ptr, edged_width, rounding);
			interpolate8x8_halfpel_v(v_ptr, n_ptr, edged_width, rounding);
			interpolate8x8_halfpel_hv(hv_ptr, n_ptr, edged_width, rounding);

			n_ptr += 8;
			h_ptr += 8;
			v_ptr += 8;
			hv_ptr += 8;
		}
		h_ptr += stride_add;
		v_ptr += stride_add;
		hv_ptr += stride_add;
		n_ptr += stride_add;
	}
}

/***************************************************************************************
 **  Parallel Code . By gary
 ***************************************************************************************/
void
OneMV_image_interpolate(const IMAGE * refn,
						int32_t x,
						int32_t y,
				        IMAGE * refh,
				        IMAGE * refv,
				        IMAGE * refhv,
				        uint32_t edged_width,
				        uint32_t edged_height,
				        uint32_t rounding)
{
	uint8_t *n_ptr, *h_ptr, *v_ptr, *hv_ptr;
    n_ptr = refn->y;
	h_ptr = refh->y;
	v_ptr = refv->y;
	hv_ptr = refhv->y;

	n_ptr = n_ptr + y*edged_width + x;

	OneMV_interpolate8x8_halfpel_h(h_ptr, n_ptr-1, edged_width, rounding);
	OneMV_interpolate8x8_halfpel_v(v_ptr, n_ptr-edged_width, edged_width, rounding);
    OneMV_interpolate8x8_halfpel_hv(hv_ptr, n_ptr-edged_width-1, edged_width, rounding);
}

#define _MSC_VER
#if defined(_MSC_VER)
#include <math.h>

float
image_psnr(IMAGE * orig_image,
		   IMAGE * recon_image,
		   uint16_t stride,
		   uint16_t width,
		   uint16_t height)
{
	int32_t diff, x, y, quad = 0;
	uint8_t *orig = orig_image->y;
	uint8_t *recon = recon_image->y;
	float psnr_y;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			diff = *(orig + x) - *(recon + x);
			quad += diff * diff;
		}
		orig += stride;
		recon += stride;
	}

	psnr_y = (float) quad / (float) (width * height);

	if (psnr_y) {
		psnr_y = (float) (255 * 255) / psnr_y;
		psnr_y = 10 * (float) log10(psnr_y);
	} else
		psnr_y = (float) 99.99;

	return psnr_y;
}

#endif

