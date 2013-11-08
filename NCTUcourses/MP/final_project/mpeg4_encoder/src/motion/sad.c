#include "../portab.h"
#include "sad.h"

sad16FuncPtr sad16;
sad8FuncPtr sad8;
dev16FuncPtr dev16;

sadInitFuncPtr sadInit;

#define ABS(X) (((X)>0)?(X):-(X))


uint32_t
sad16_c(const uint8_t * const cur,
		const uint8_t * const ref,
		const uint32_t stride,
		const uint32_t best_sad)
{

	uint32_t sad = 0;
	uint32_t i, j;
	uint8_t const *ptr_cur = cur;
	uint8_t const *ptr_ref = ref;

	for (j = 0; j < 16; j++) {

		for (i = 0; i < 16; i++) {

			sad += ABS(*(ptr_cur + i) - *(ptr_ref + i));

			if (sad >= best_sad) {
				return sad;
			}


		}

		ptr_cur += stride;
		ptr_ref += stride;

	}

	return sad;

}


uint32_t
sad8_c(const uint8_t * const cur,
	   const uint8_t * const ref,
	   const uint32_t stride)
{
	uint32_t sad = 0;
	uint32_t i, j;
	uint8_t const *ptr_cur = cur;
	uint8_t const *ptr_ref = ref;

	for (j = 0; j < 8; j++) {

		for (i = 0; i < 8; i++) {
			sad += ABS(*(ptr_cur + i) - *(ptr_ref + i));
		}

		ptr_cur += stride;
		ptr_ref += stride;

	}

	return sad;
}




/* average deviation from mean */

uint32_t
dev16_c(const uint8_t * const cur,
		const uint32_t stride)
{

	uint32_t mean = 0;
	uint32_t dev = 0;
	uint32_t i, j;
	uint8_t const *ptr_cur = cur;

	for (j = 0; j < 16; j++) {

		for (i = 0; i < 16; i++)
			mean += *(ptr_cur + i);

		ptr_cur += stride;

	}

	mean /= (16 * 16);
	ptr_cur = cur;

	for (j = 0; j < 16; j++) {

		for (i = 0; i < 16; i++)
			dev += ABS(*(ptr_cur + i) - (int32_t) mean);

		ptr_cur += stride;

	}

	return dev;
}
